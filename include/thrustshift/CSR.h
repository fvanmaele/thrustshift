#pragma once

#include <memory_resource>
#include <vector>

#include <gsl-lite/gsl-lite.hpp>

#include <cuda/define_specifiers.hpp>

#include <thrustshift/memory-resource.h>

namespace thrustshift {

//! Col indices must be ordered
template <typename DataType, typename IndexType>
class CSR {
   public:
	using value_type = DataType;
	using index_type = IndexType;

   private:
	pmr::managed_resource_type default_resource_;

	bool cols_are_sorted() {
		for (size_t row_id = 0; row_id < this->num_rows(); ++row_id) {
			if (!std::is_sorted(col_indices_.begin() + row_ptrs_[row_id],
			                    col_indices_.begin() + row_ptrs_[row_id + 1])) {
				return false;
			}
		}
		return true;
	}

   public:
	CSR() : row_ptrs_(1, 0, &default_resource_), num_cols_(0) {
	}

	template <class DataRange,
	          class ColIndRange,
	          class RowPtrsRange,
	          class MemoryResource>
	CSR(DataRange&& values,
	    ColIndRange&& col_indices,
	    RowPtrsRange&& row_ptrs,
	    size_t num_cols,
	    MemoryResource& memory_resource)
	    : values_(values.begin(), values.end(), &memory_resource),
	      col_indices_(col_indices.begin(),
	                   col_indices.end(),
	                   &memory_resource),
	      row_ptrs_(row_ptrs.begin(), row_ptrs.end(), &memory_resource),
	      num_cols_(num_cols) {
		gsl_Expects(values.size() == col_indices.size());
		gsl_Expects(row_ptrs.size() > 0);
		gsl_Expects(row_ptrs[0] == 0);
		gsl_ExpectsAudit(cols_are_sorted());
	}

	template <class DataRange, class ColIndRange, class RowPtrsRange>
	CSR(DataRange&& values,
	    ColIndRange&& col_indices,
	    RowPtrsRange&& row_ptrs,
	    size_t num_cols)
	    : CSR(std::forward<DataRange>(values),
	          std::forward<ColIndRange>(col_indices),
	          std::forward<RowPtrsRange>(row_ptrs),
	          num_cols,
	          default_resource_) {
	}

	// The copy constructor is declared explicitly to ensure
	// managed memory is used per default.
	CSR(const CSR& other)
	    : CSR(other.values(),
	          other.col_indices(),
	          other.row_ptrs(),
	          other.num_cols()) {
	}

	CSR(CSR&& other) = default;

	gsl_lite::span<DataType> values() {
		return gsl_lite::make_span(values_);
	}

	gsl_lite::span<const DataType> values() const {
		return gsl_lite::make_span(values_);
	}

	gsl_lite::span<IndexType> col_indices() {
		return gsl_lite::make_span(col_indices_);
	}

	gsl_lite::span<const IndexType> col_indices() const {
		return gsl_lite::make_span(col_indices_);
	}

	gsl_lite::span<IndexType> row_ptrs() {
		return gsl_lite::make_span(row_ptrs_);
	}

	gsl_lite::span<const IndexType> row_ptrs() const {
		return gsl_lite::make_span(row_ptrs_);
	}

	size_t num_rows() const {
		return row_ptrs_.size() - 1;
	}

	size_t num_cols() const {
		return num_cols_;
	}

   private:
	std::pmr::vector<DataType> values_;
	std::pmr::vector<IndexType> col_indices_;
	std::pmr::vector<IndexType> row_ptrs_;
	size_t num_cols_;
};

template <typename DataType, typename IndexType>
class CSR_view {

   public:

	using value_type = DataType;
	using index_type = IndexType;

	template <typename OtherDataType, typename OtherIndexType>
	CSR_view(CSR<OtherDataType, OtherIndexType>& owner)
	    : values_(owner.values()),
	      col_indices_(owner.col_indices()),
	      row_ptrs_(owner.row_ptrs()),
	      num_cols_(owner.num_cols()) {
	}

	template <typename OtherDataType, typename OtherIndexType>
	CSR_view(const CSR<OtherDataType, OtherIndexType>& owner)
	    : values_(owner.values()),
	      col_indices_(owner.col_indices()),
	      row_ptrs_(owner.row_ptrs()),
	      num_cols_(owner.num_cols()) {
	}

	CSR_view(const CSR_view& other) = default;
	CSR_view(CSR_view&& other) = default;

	CUDA_FHD gsl_lite::span<DataType> values() {
		return values_;
	}

	CUDA_FHD gsl_lite::span<const DataType> values() const {
		return values_;
	}

	CUDA_FHD gsl_lite::span<IndexType> col_indices() {
		return col_indices_;
	}

	CUDA_FHD gsl_lite::span<const IndexType> col_indices() const {
		return col_indices_;
	}

	CUDA_FHD gsl_lite::span<IndexType> row_ptrs() {
		return row_ptrs_;
	}

	CUDA_FHD gsl_lite::span<const IndexType> row_ptrs() const {
		return row_ptrs_;
	}

	CUDA_FHD size_t num_rows() const {
		return row_ptrs_.size() - 1;
	}

	CUDA_FHD size_t num_cols() const {
		return num_cols_;
	}

   private:
	gsl_lite::span<DataType> values_;
	gsl_lite::span<IndexType> col_indices_;
	gsl_lite::span<IndexType> row_ptrs_;
	size_t num_cols_;
};

} // namespace thrustshift
