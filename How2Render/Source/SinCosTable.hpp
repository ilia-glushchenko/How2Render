#pragma once

struct SinCosTable
{
	float *pSin = nullptr;
	float *pCos = nullptr;
};

std::tuple<bool, SinCosTable> CreateSinCosTable(float start, float step, uint32_t count)
{
	SinCosTable table;

	table.pSin = new float[count];
	if (!table.pSin)
		return {false, table};
	table.pCos = new float[count];
	if (!table.pCos)
	{
		delete[] table.pSin;
		table.pSin = nullptr;
		return {false, table};
	}

	float angle = start;
	for (uint32_t i = 0; i < count; ++i)
	{
		XMScalarSinCosEst(&table.pSin[i], &table.pCos[i], angle);
		angle += step;
	}

	return {true, table};
}

void CleanupSinCosTable(SinCosTable& table)
{
	if (table.pSin)
	{
		delete[] table.pSin;
		table.pSin = nullptr;
	}
	if (table.pCos)
	{
		delete[] table.pCos;
		table.pCos = nullptr;
	}
}
