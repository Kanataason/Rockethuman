#include "ScoreManager.h"
#include "ObjectStatus.h"

void ScoreManager::SubScribeEvent(ObjectStatus* status)
{
	status->OnSendScore.Subscribe([this](float score) {AddScore(score);});
}
