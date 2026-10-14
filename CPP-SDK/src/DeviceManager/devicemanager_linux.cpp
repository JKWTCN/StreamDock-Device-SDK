#include "devicemanager.h"
#if __linux__

#include <chrono>
#include <thread>
#include <unordered_set>
#include <vector>

void DeviceManager::listen(std::function<void(std::shared_ptr<StreamDock>)> connect_and_run)
{
	if (isListening_)
		return;
	isListening_ = true;
	listener_ = std::thread([this, connect_and_run]()
		{
			// Poll HIDAPI paths; libusb paths are not /dev/hidraw device nodes.
			while (isListening_) {
				DeviceEnumerator::instance().enumerate();
				std::unordered_set<std::string> currentPaths;
				for (const auto& device : DeviceEnumerator::instance().currDevices()) {
					if (StreamDockFactory::instance().exist(device->_vendor_id, device->_product_id) &&
						StreamDock::isStreamDockHidDeviceUsage(device->toPureHidDeviceInfo()))
						currentPaths.insert(device->_path);
				}

				std::unordered_set<std::string> previousPaths;
				{
					std::lock_guard<std::mutex> lock(streamdocksMutex_);
					for (const auto& entry : streamdocks_)
						previousPaths.insert(entry.first);
				}

				if (currentPaths != previousPaths) {
					enumerator();
					std::vector<std::shared_ptr<StreamDock>> addedDevices;
					{
						std::lock_guard<std::mutex> lock(streamdocksMutex_);
						for (const auto& entry : streamdocks_) {
							if (previousPaths.find(entry.first) == previousPaths.end())
								addedDevices.push_back(entry.second);
						}
					}
					// Callbacks may access the manager, so invoke them outside its lock.
					for (const auto& device : addedDevices) {
						if (connect_and_run)
							connect_and_run(device);
					}
				}
				std::this_thread::sleep_for(std::chrono::milliseconds(500));
			}
		});
}

void DeviceManager::stopListen()
{
	isListening_ = false;
	if (listener_.joinable())
		listener_.join();
}

#endif
