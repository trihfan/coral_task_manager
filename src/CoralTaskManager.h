#pragma once

// config
#include "Config.h"

// includes
#include "Task.h"
#include "TaskManager.h"

// Usage
/*
 *  coral::taskmanager::Start();
 *
 *  Task* task = coral::taskmanager::CreateTask([](task, userData){ ... });
 *  coral::taskmanager::Run(task);
 *  coral::taskmanager::Wait(task);
 * 
 *  coral::taskmanager::Stop();
 */