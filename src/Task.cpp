#include "Task.h"
#include "TaskBuffer.h"

using namespace coral::taskmanager;

TaskHandle::operator bool() const
{
    return id != 0;
}

TaskData* TaskHandle::operator->()
{
    return TaskBuffer::Get(id);
}

TaskData* TaskHandle::operator*()
{
    return TaskBuffer::Get(id);
}