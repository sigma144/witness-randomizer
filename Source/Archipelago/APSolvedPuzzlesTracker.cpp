#include "APSolvedPuzzlesTracker.h"

void APSolvedPuzzles::action() {
	std::list<int64_t> solvedLocations;

	auto it = panelIdToLocationId.begin();
	while (it != panelIdToLocationId.end())
	{
		int panelId = it->first;
		int locationId = it->second;

		if (ReadPanelData<int>(panelId, SOLVED))
		{
			solvedLocations.push_back(locationId);

			it = panelIdToLocationId.erase(it);
		}
		else
		{
			it++;
		}
	}

	if(!solvedLocations.empty())
		ap->LocationChecks(solvedLocations);
}

void APSolvedPuzzles::MarkLocationChecked(int locationId)
{
	eraseByLocationId(locationId);
}


void APSolvedPuzzles::eraseByLocationId(int value)
{
	auto it = panelIdToLocationId.begin();
	while (it != panelIdToLocationId.end())
	{
		if (it->second == value)
		{
			it = panelIdToLocationId.erase(it);
		}
		else
		{
			it++;
		}
	}
}