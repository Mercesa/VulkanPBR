#include "QueryPoolVulkan.h"



QueryPoolVulkan::QueryPoolVulkan()
{
}


QueryPoolVulkan::~QueryPoolVulkan()
{
}



void QueryPoolVulkan::Create(const vk::Device& iDevice,
	const uint32_t& iAmountOfQueries,
	vk::QueryType iQueryTypes, vk::QueryPipelineStatisticFlags iPipelineFlags)
{

}

void QueryPoolVulkan::Destroy(const vk::Device& iDevice)
{
	iDevice.destroyQueryPool(pool);
}