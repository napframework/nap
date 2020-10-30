/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/. */

#pragma once

namespace nap
{
	/** 
	 * Builds an ObjectGraph based on a list of ITEMS. Each item returns what other item it points to through ITEM::GetPointees().
	 * After calling ObjectGraph::build(), the nodes and edges of the graph can be traversed. Each Node contains an item that
	 * can be inspected.
	 * The template parameter ITEM represent the user data that is stored in a node. The user data can contain any data that is 
	 * required for a node. ITEM::Type refers to the internal type that you are wrapping in the ITEM structure. This is also the 
	 * type that is used a input for the build() function.
	 * ITEMS must be constructed using a creation function that is passed to build(). This way, additional user data can be passed
	 * onto the internal items.
	 */
	template<typename ITEM>
	class ObjectGraph final
	{
	public:
		struct Node;

		using ItemList = std::vector<typename ITEM::Type>;
		using ItemCreationFunction = std::function<ITEM(typename ITEM::Type)>;

		/**
		* Represent an 'edge' in the ObjectGraph. This is a link from an object to either a file or another object.
		*/
		struct Edge
		{
			Node*		mSource = nullptr;		// Link source
			Node*		mDest = nullptr;		// Link target
		};

		/*
		* Represents a node in the ObjectGraph, containing an ITEM.
		*/
		struct Node
		{
			int					mDepth = -1;			// Depth of the node is calculated during build, it represents at what level from the root this node is.
			ITEM				mItem;					// User data per Node
			std::vector<Edge*>	mIncomingEdges;			// List of incoming edges
			std::vector<Edge*>	mOutgoingEdges;			// List of outgoing edges
		};

		/*
		 * Rebuilds the object graph. If building fails, return false. If the object graph was previously built, this will also clear the previous state 
		 * @param objectList : list of objects to build the graph from.
		 * @param errorState: if false is returned, contains error information.
		 */
		bool build(const ItemList& objectList, const ItemCreationFunction& creationFunction, utility::ErrorState& errorState)
		{
			// Traverse graph and build nodes and edges
			for (typename ITEM::Type object : objectList)
				if (getOrCreateItemNode(creationFunction(object)) == nullptr)
					return false;

			return rebuild(errorState);
		}

		/*
		* Rebuilds the object graph. If building fails, return false. If the object graph was previously built, this will not clear the previous state.
		*/
		bool rebuild(utility::ErrorState& errorState)
		{
			mEdges.clear();
			for (auto& kvp : mNodes)
			{
				kvp.second->mIncomingEdges.clear();
				kvp.second->mOutgoingEdges.clear();
			}

			for (auto& kvp : mNodes)
			{
				std::vector<ITEM> pointees;
				if (!kvp.second->mItem.getPointees(pointees, errorState))
					return false;

				for (ITEM& pointee_node : pointees)
				{
					Edge* edge = new Edge();
					edge->mSource = kvp.second.get();
					edge->mDest = getOrCreateItemNode(pointee_node);
					edge->mDest->mIncomingEdges.push_back(edge);
					edge->mSource->mOutgoingEdges.push_back(edge);
					mEdges.push_back(std::unique_ptr<Edge>(edge));
				}
			}

			// Assign graph depth
			std::vector<Node*> root_nodes;
			for (auto& kvp : mNodes)
			{
				Node* node = kvp.second.get();
				node->mDepth = -1;
				if (node->mIncomingEdges.empty())
					root_nodes.push_back(node);
			}

			for (Node* root_node : root_nodes)
				assignDepthRecursive(root_node, 0);

			return true;
		}

		/*
		 * Visit all nodes in the graph and call the visitor function
		 *
		 * @param visitor The visitor to be called for each node in the graph
		 */
		void visitNodes(const std::function<void(const Node&)>& visitor) const
		{
			std::vector<Node*> result;
			for (auto& kvp : mNodes)
				visitor(*kvp.second);
		}

		/*
		 * Returns object graph node.
		 */
		Node* findNode(const std::string& ID) const
		{
			typename NodeMap::const_iterator iter = mNodes.find(ID);
			if (iter == mNodes.end())
				return nullptr;
			
			return iter->second.get();
		}

		/*
		 * Returns all nodes that are processed during build, sorted on graph depth. 
		 */
		const std::vector<Node*> getSortedNodes() const
		{
			std::vector<Node*> sorted_nodes;
			sorted_nodes.reserve(mNodes.size());
			for (auto& kvp : mNodes)
				sorted_nodes.push_back(kvp.second.get());

			std::sort(sorted_nodes.begin(), sorted_nodes.end(), [](Node* nodeA, Node* nodeB) { return nodeA->mDepth > nodeB->mDepth; });

			return sorted_nodes;
		}


		/**
		 * Walks object graph by traversing incoming edges and pushing the results in an array.
		 * @param node: start node to traverse incoming edges from.
		 * @param incomingObjects: output of the function.
		 */
		static void addIncomingObjectsRecursive(Node* node, std::set<Node*>& incomingObjects)
		{
			if (incomingObjects.find(node) != incomingObjects.end())
				return;

			incomingObjects.insert(node);

			for (Edge* incoming_edge : node->mIncomingEdges)
				addIncomingObjectsRecursive(incoming_edge->mSource, incomingObjects);
		}

	private:

		/**
		* Recursively scans outgoing edges of nodes and increments the depth for each level deeper.
		* We try to avoid rescanning parts of a graph that have already been processed, but sometimes
		* we must revisit a part of the graph to make sure that the depth is accurate. For example:
		*
		*	A ---> B ----------> C
		*    \                /
		*     \---> D ---> E -
		*
		* In this situation, if the A-B-C branch is visited first, C will have depth 3, but it will
		* be corrected when processing branch A-D-E-C: object E will have depth 3, which is >= object C's depth.
		* So, the final depth will become:
		* 
		*	A(1) ---> B(2) ----------> C(4)
		*    \                      /
		*     \---> D(2) ---> E(3) -
		*
		*/
		void assignDepthRecursive(Node* node, int depth)
		{
			// The following is both a test for 'is visited' and 'should we revisit':
			if (node->mDepth >= depth)
				return;

			for (Edge* outgoing_edge : node->mOutgoingEdges)
				assignDepthRecursive(outgoing_edge->mDest, depth + 1);

			node->mDepth = depth;
		}


		Node* getOrCreateItemNode(const ITEM& item)
		{
			Node* result = nullptr;
			typename NodeMap::iterator iter = mNodes.find(item.getID());
			if (iter == mNodes.end())
			{
				auto node = std::make_unique<Node>();
				node->mItem = item;
				result = node.get();
				mNodes.insert(std::make_pair(item.getID(), std::move(node)));
			}
			else
			{
				result = iter->second.get();
			}

			return result;
		}

	private:
		using NodeMap = std::map<std::string, std::unique_ptr<Node>>;
		NodeMap								mNodes;		// All nodes in the graph, mapped from ID to node
		std::vector<std::unique_ptr<Edge>>	mEdges;		// All edges in the graph
	};
}
