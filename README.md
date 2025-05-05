# Araneid
Araneid is a network simulator easy for developers to test applications in different network conditions. It builds simulated network and bridges LXC containers to the network. 

Simply run your applications in LXC containers created in right way and packets would be transmitted!

P.S. "araneid" is from "araneidae", a spider family in which the members are beautiful.

## Pre-requisites
1. LXC library installed, you can install it by:
   ```bash
   sudo apt install liblxc1 liblxc-dev
   ```
2. Your applications to run in LXC containers.
3. The shell scripts to launch your applications in LXC containers, which should be executable.
4. Put your applications and scripts in the `desktop/` directory. Araneid will find them.

## Explanation
Araneid provides an environment for developers to configure different network conditions, and the packets are produced and consumed by the applications of their own. 

You might be curious about how packets are delivered between your applications and araneid. TAP device is the answer, because it is the bridge between the network and your applications. But it's a little bit troublesome to set up TAP devices and launch LXC containers before running araneid, I haven't found a better way to do it. So do it by yourself please.

## Usage
> 2025-05-05: I haven't write a demo yet, just build the static library and use it please. Later I will write a demo to show how to use it.

```shell
make build && cmake .. && make 
```

You can find the static library in `build/libaraneid.a`. You can use it in your own project.
