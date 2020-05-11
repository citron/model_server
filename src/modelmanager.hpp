//*****************************************************************************
// Copyright 2020 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//*****************************************************************************
#pragma once

#include <future>
#include <map>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/istreamwrapper.h>

#include "model.hpp"

namespace ovms {
    /**
     * @brief Model manager is managing the list of model topologies enabled for serving and their versions.
     */
    class ModelManager {
    private:
        /**
         * @brief A default constructor is private
         */
        ModelManager() = default;

        /**
         * @brief Private copying constructor
         */
        ModelManager(const ModelManager&);

        /**
         * @brief Reads models from configuration file
         * 
         * @param jsonFilename configuration file
         * @return Status 
         */
        Status loadConfig(const std::string& jsonFilename);

        /**
         * @brief Watcher thread for monitor changes in config
         */
        void watcher(std::future<void> exit);

        /**
         * @brief Watcher interval for checking changes in config
         */
        static const uint watcherIntervalSec;

        /**
         * @brief A JSON configuration filename
         */
        std::string configFilename;

        /**
         * @brief A collection of models
         */
        std::map<std::string, std::shared_ptr<Model>> models;

        /**
         * @brief A thread object used for monitoring changes in config
         */
        std::thread monitor;

        /**
         * @brief An exit signal to notify watcher thread to exit
         */
        std::promise<void> exit;

    public:
        /**
         * @brief Gets the instance of ModelManager
         */
        static ModelManager& getInstance() {
            static ModelManager instance;

        return instance;
        }

        /**
         * @brief Gets config filename
         * 
         * @return config filename
         */
        const std::string& getConfigFilename() {
            return configFilename;
        }

        /**
         * @brief Gets models collection
         * 
         * @return models collection
         */
        const std::map<std::string, std::shared_ptr<Model>>& getModels() {
            return models;
        }

        /**
         * @brief Finds model with specific name
         *
         * @param name of the model to search for
         *
         * @return pointer to Model or nullptr if not found 
         */
        const std::shared_ptr<Model> findModelByName(const std::string& name) const {
            auto it = models.find(name);
            return it != models.end() ? it->second : nullptr;
        }

        /**
         * @brief Finds model instance with specific name and version, returns default if version not specified
         *
         * @param name of the model to search for
         * @param version of the model to search for or 0 if default
         *
         * @return pointer to ModelInstance or nullptr if not found 
         */
        const std::shared_ptr<ModelInstance> findModelInstance(const std::string& name, model_version_t version = 0) {
            auto model = findModelByName(name);
            if (!model) {
                return nullptr;
            }

            if (version == 0) {
                return model->getDefaultModelInstance();
            } else {
                return model->getModelInstanceByVersion(version);
            }
        }

        /**
         * @brief Starts model manager using provided config file
         * 
         * @param filename
         * @return status
         */
        Status start(const std::string& jsonFilename);

        /**
         * @brief Load model versions located in base path
         * 
         * @param string base path, where versions are located
         * @param ModelConfig config
         * @return status
         */
        Status loadModelWithVersions(
            const   std::string& basePath,
                    ModelConfig& config);

        /**
         * @brief Starts model manager using ovms::Config
         * 
         * @return status
         */
        Status start();

        /**
         * @brief Gracefully finish the thread
         */
        void join();

        /**
         * @brief Parses json node for plugin config keys and values
         * 
         * @param json node representing plugin_config
         * @param target config reference where parsed data will be stored
         * @return status
         */
        static Status parsePluginConfig(const rapidjson::Value& node, plugin_config_t& config);

        /**
         * @brief Parses string for plugin config keys and values
         * 
         * @param string representing plugin_config
         * @param target config reference where parsed data will be stored
         * @return status
         */
        static Status parsePluginConfig(std::string command, plugin_config_t& config);

        /**
         * @brief Iterates over directories in specific path and returns available model versions
         * 
         * @param string path where to look for versions
         * @param reference to container of available model version results
         * @return operation status
         */
        static Status readAvailableVersions(const std::string& path, std::vector<model_version_t>& versions);

        /**
         * @brief  Parses mapping_config.json for mapping input/outputs in the model
         * 
         * @param base 
         * @param mappingInputs 
         * @param mappingOutputs 
         */
        void parseModelMapping(const std::string& base, mapping_config_t& mappingInputs, mapping_config_t& mappingOutputs);
    };
}  // namespace ovms
