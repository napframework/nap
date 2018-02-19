#pragma once
#include <QSortFilterProxyModel>
#include <QSet>

namespace napkin
{
	/**
	 * A variation on QSortFilterProxyModel.
	 * When filtering, this model includes matches in a deeper levels of the hierarchy.
	 */
	class LeafFilterProxyModel : public QSortFilterProxyModel
	{
	public:
		LeafFilterProxyModel();

		/**
		 * Exclude an index from filtering, causing it to show up even when the filter wouldn't normally show it.
		 * @param sourceIndex The source index to be exempted from filtering.
		 */
		void exemptSourceIndex(QModelIndex sourceIndex);

		/**
		 * Clear the list of exempted indexes set using exemptSourceIndex()
		 */
		void clearExemptions();

	protected:
		bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

	private:
		bool acceptsAnyChild(int sourceRow, QModelIndex sourceParent) const;

		QSet<QModelIndex> mExemptions;
	};
};