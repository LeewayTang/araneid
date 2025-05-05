#ifndef ARANEID_SYSTEM_LXC_MANAGER_HPP
#define ARANEID_SYSTEM_LXC_MANAGER_HPP

#include <lxc/lxccontainer.h>

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace araneid {

// Pure static class to manage the virtual machine.
// It must be executed with root privilege.
class MachineManager {
 public:
  // Singleton instance of MachineManager.
  static MachineManager& Instance() {
    static MachineManager instance;
    return instance;
  }
  MachineManager(const MachineManager&) = delete;
  MachineManager& operator=(const MachineManager&) = delete;
  MachineManager(MachineManager&&) = delete;
  MachineManager& operator=(MachineManager&&) = delete;

  // Before creating a machine, related bridge and tap device will be created.
  // The default template is "download" and the default args are
  // "--dist", "ubuntu".
  // The name of the machine must be unique, consisting of lowercase
  // letters.
  // Config example:
  // config["lxc.uts.name"] = name
  // config["lxc.net.0.type"] = "veth"
  // config["lxc.net.0.flag"] = "up"
  // config["lxc.net.0.link"] = "br-" + name
  // config["lxc.net.0.ipv4.address"] = "10.0.0.1/24"
  // config["lxc.mount.entry"] = "/path/to/container/ /opt/ none bind,create=dir
  // 0 0"
  // Note that the config["lxc.mount.entry"] directory will contain all files
  // you need in this LXC, and `run.sh` will be executed in this directory.
  static bool CreateMachine(std::string name,
                            std::unordered_map<std::string, std::string> config,
                            std::string template_name = "download",
                            std::vector<std::string> template_args = {
                                "--dist", "ubuntu"});
  // Before starting a machine, the related network bridge and tap
  // device will be set up.
  //
  static void StartMachines();

 private:
  MachineManager();
  // It's a simple encapsulation of the Linux container (LXC) management.
  class LinuxContainer {
   public:
    explicit LinuxContainer(std::string name);
    void SetConfig(const std::string& key, const std::string& value);
    void Create(const std::string& template_name,
                std::vector<std::string> template_args);
    void Start();

   private:
    std::shared_ptr<lxc_container> container_;
  };
  std::unordered_map<std::string, std::shared_ptr<LinuxContainer>> containers_;
};

}  // namespace araneid

#endif  // ARANEID_SYSTEM_LXC_MANAGER_HPP