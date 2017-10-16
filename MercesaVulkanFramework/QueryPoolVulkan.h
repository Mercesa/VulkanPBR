#pragma once

#include "RenderingIncludes.h"


class QueryPoolVulkan
{
public:
	QueryPoolVulkan();
	~QueryPoolVulkan();

	void Create(const vk::Device& iDevice, 
		const uint32_t& iAmountOfQueries, 
		vk::QueryType iQueryTypes, vk::QueryPipelineStatisticFlags iPipelineFlags);
	void Destroy(const vk::Device& iDevice);

	vk::QueryPool pool;
	uint32_t amountOfQueries;
	vk::QueryType typeOfQueries;

	// for if there are any pipelines this query pool needs to take care off
	vk::QueryPipelineStatisticFlagBits pipelineFlags;
	
};

