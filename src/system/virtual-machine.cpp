#include "virtual-machine.hpp"

#include "base/log.hpp"
namespace araneid {

MachineManager::LinuxContainer::LinuxContainer(std::string name) {
  container_ =
      std::shared_ptr<lxc_container>(lxc_container_new(name.c_str(), nullptr));
  if (!container_) {
    ALOG_ERROR << "Failed to create LXC container: " << name;
  }
}

void MachineManager::LinuxContainer::SetConfig(const std::string& key,
                                               const std::string& value) {
  if (!container_->set_config_item(container_.get(), key.c_str(),
                                   value.c_str())) {
    ALOG_ERROR << "Failed to set config item: " << key << " = " << value;
  }
}

void MachineManager::LinuxContainer::Create(
    const std::string& template_name, std::vector<std::string> template_args) {
  char* args[template_args.size() + 1];
  for (size_t i = 0; i < template_args.size(); ++i) {
    args[i] = const_cast<char*>(template_args[i].c_str());
  }
  if (!container_->create(container_.get(), template_name.c_str(), nullptr,
                          nullptr, LXC_CREATE_QUIET, args)) {
    ALOG_ERROR << "Failed to create LXC container: " << container_->name;
  }
}

void MachineManager::LinuxContainer::Start() {
  const char* args[] = {"/opt/run.sh", NULL};
  if (!container_->start(container_.get(), 0, const_cast<char**>(args))) {
    ALOG_ERROR << "Failed to start LXC container: " << container_->name;
  }
}

MachineManager::MachineManager() {
  if (system("modprobe br_netfilter") != 0) {
    ALOG_ERROR << "Failed to load br_netfilter module";
  }
}

bool MachineManager::CreateMachine(
    std::string name, std::unordered_map<std::string, std::string> config,
    std::string template_name, std::vector<std::string> template_args) {
  // crate network bridge
  if (system(("ip link add br-" + name + " type bridge").c_str()) != 0) {
    ALOG_ERROR << "Failed to create network bridge: br-" << name;
    return false;
  }
  // create tap device
  if (system(("ip tuntap add mode tap tap-" + name).c_str()) != 0) {
    ALOG_ERROR << "Failed to create tap device: tap-" << name;
    return false;
  }
  // set tap device to promisc mode
  if (system(("ip link set dev tap-" + name + " promisc on up").c_str()) != 0) {
    ALOG_ERROR << "Failed to set tap device to promisc mode: tap-" << name;
    return false;
  }
  // connect tap device to bridge
  if (system(("ip link set dev tap-" + name + " master br-" + name).c_str()) !=
      0) {
    ALOG_ERROR << "Failed to connect tap device to bridge: tap-" << name;
    return false;
  }
  // set bridge to up
  if (system(("ip link set dev br-" + name + " up").c_str()) != 0) {
    ALOG_ERROR << "Failed to set bridge to up: br-" << name;
    return false;
  }

  auto container = std::make_shared<LinuxContainer>(name);
  for (const auto& [key, value] : config) {
    container->SetConfig(key, value);
  }
  container->Create(template_name, template_args);
  container->Start();
  Instance().containers_[name] = container;
  return true;
}

void MachineManager::StartMachines() {
  if (system("sh -c 'pushd /proc/sys/net/bridge && echo 0 | tee bridge-nf-* && "
             "popd'") != 0) {
    ALOG_ERROR << "Failed to set bridge-nf-call-iptables to 0";
  }
  for (const auto& [name, container] : Instance().containers_) {
    container->Start();
  }
}

}  // namespace araneid