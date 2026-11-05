#pragma once

#define NS_TIMESTAMP_LOG_INFO(message) NS_LOG_INFO("At time " << Simulator::Now().As(Time::S) << " " << message)
#define NS_TIMESTAMP_LOG_DEBUG(message) NS_LOG_DEBUG("At time " << Simulator::Now().As(Time::S) << " " << message)
