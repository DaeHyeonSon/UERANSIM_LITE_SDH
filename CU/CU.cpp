//
// Created by Hoonyong Park on 5/10/23.
//

#include "CU.hpp"
#include "app/task.hpp"
#include "f1ap/task.hpp"
#include "rrc/task.hpp"

#include <lib/app/cli_base.hpp>

namespace nr::CU
{

CentralUnit::CentralUnit(CUConfig *config, app::INodeListener *nodeListener, NtsTask *cliCallbackTask)
{
    auto *base = new TaskBase();
    base->config = config;
    base->logBase = new LogBase("logs/" + config->name + ".log");
    base->nodeListener = nodeListener;
    base->cliCallbackTask = cliCallbackTask;

    base->appTask = new CUAppTask(base);
    base->sctpServerTask = new SctpServerTask(base);
    base->rrcTask = new CURrcTask(base);
    base->f1apTask = new F1apTask(base);

    taskBase = base;
}

CentralUnit::~CentralUnit()
{
    taskBase->appTask->quit();
    taskBase->sctpServerTask->quit();
    taskBase->rrcTask->quit();
    taskBase->f1apTask->quit();

    delete taskBase->appTask;
    delete taskBase->sctpServerTask;
    delete taskBase->rrcTask;
    delete taskBase->f1apTask;

    delete taskBase->logBase;

    delete taskBase;
}

void CentralUnit::start()
{
    taskBase->appTask->start();
    taskBase->sctpServerTask->start();
    taskBase->rrcTask->start();
    taskBase->f1apTask->start();
}

void CentralUnit::pushCommand(std::unique_ptr<app::CUCliCommand> cmd, const InetAddress &address)
{
    taskBase->appTask->push(std::make_unique<NmCUCliCommand>(std::move(cmd), address));
}

} // namespace nr::CU