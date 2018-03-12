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
		using FilterFunction = std::function<bool(const LeafFilterProxyModel& model, int sourceRow, const QModelIndex& sourceParent)>;

		LeafFilterProxyModel();

		/**
		 * Exclude an index from filtering, causing it to show up even when the filter wouldn't normally show it.
		 * @param sourceIndex The source index to be exempted from filtering.
		 */
		void exemptSourceIndex(QModelIndex sourceIndex);

		/**
		 * Add a filter to the model
		 */
		void addExtraFilter(FilterFunction func);

		/**
		 * Clear the list of exempted indexes set using exemptSourceIndex()
		 */
		void clearExemptions();

		/**
		 * Forward to Qt's protected invalidateFilter() method.
		 */
		void refreshFilter() { invalidateFilter(); }

	protected:
		bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

	private:
		bool acceptsAnyChild(int sourceRow, QModelIndex sourceParent) const;

		bool isExempt(int sourceRow, const QModelIndex& sourceParent) const;

		QSet<QModelIndex> mExemptions;
		QList<FilterFunction> mExtraFilters;
	};
};