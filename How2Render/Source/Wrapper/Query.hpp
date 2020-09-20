#pragma once

#include "Wrapper/Context.hpp"
#include <cstdio>
#include <d3d11.h>

namespace h2r
{

	struct PerformaceQueries
	{
		ID3D11Query *timestampStartQuery = nullptr;
		ID3D11Query *timestampEndQuery = nullptr;
		ID3D11Query *disjointQuery = nullptr;
	};

	inline ID3D11Query *CreateQuery(Context const &context, D3D11_QUERY queryType)
	{
		ID3D11Query *query = nullptr;

		D3D11_QUERY_DESC desc;
		desc.Query = queryType;
		desc.MiscFlags = 0;
		auto hr = context.pd3dDevice->CreateQuery(&desc, &query);
		if (FAILED(hr))
		{
			printf("Failed to create performance query\n");
			return nullptr;
		}

		return query;
	}

	inline void CleanupQuery(ID3D11Query *query)
	{
		if (query != nullptr)
		{
			query->Release();
		}
	}

	inline PerformaceQueries CreatePerformanceQueries(Context const &context)
	{
		PerformaceQueries queries;

		queries.disjointQuery = CreateQuery(context, D3D11_QUERY_TIMESTAMP_DISJOINT);
		queries.timestampStartQuery = CreateQuery(context, D3D11_QUERY_TIMESTAMP);
		queries.timestampEndQuery = CreateQuery(context, D3D11_QUERY_TIMESTAMP);

		return queries;
	}

	inline void CleanupPerformanceQueries(PerformaceQueries &queries)
	{
		CleanupQuery(queries.disjointQuery);
		queries.disjointQuery = nullptr;
		CleanupQuery(queries.timestampStartQuery);
		queries.timestampStartQuery = nullptr;
		CleanupQuery(queries.timestampEndQuery);
		queries.timestampEndQuery = nullptr;
	}

	inline void BeginQueryGpuTime(Context const &context, PerformaceQueries const &queries)
	{
		context.pImmediateContext->Begin(queries.disjointQuery);
		context.pImmediateContext->End(queries.timestampStartQuery);
	}

	inline void EndQueryGpuTime(Context const &context, PerformaceQueries const &queries)
	{
		context.pImmediateContext->End(queries.disjointQuery);
		context.pImmediateContext->End(queries.timestampEndQuery);
	}

	inline double BlockAndGetGpuTimeMs(Context const &context, PerformaceQueries const &queries)
	{
		D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
		while (context.pImmediateContext->GetData(queries.disjointQuery, &disjointData, queries.disjointQuery->GetDataSize(), 0) != S_OK)
			;

		uint64_t startTime = 0;
		while (context.pImmediateContext->GetData(queries.timestampStartQuery, &startTime, queries.timestampStartQuery->GetDataSize(), 0) != S_OK)
			;

		uint64_t endTime = 0;
		while (context.pImmediateContext->GetData(queries.timestampEndQuery, &endTime, queries.timestampEndQuery->GetDataSize(), 0) != S_OK)
			;

		uint64_t const delta = endTime - startTime;
		double const freq = static_cast<double>(disjointData.Frequency);

		return (delta / freq) * 1000.;
	}

} // namespace h2r
