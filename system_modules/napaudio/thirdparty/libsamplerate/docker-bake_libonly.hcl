target "x86_64" {
    dockerfile = "BuildLib.Dockerfile"
    platforms = ["linux/amd64"]
    args = {"image_name": "x86_64-ubuntu_2004",
    "from_builder":"ubuntu:20.04",
    "extra_installs":"python-is-python3"}
    output = ["./linux/x86_64"]
}

target "armhf" {
    dockerfile = "BuildLib.Dockerfile"
    platforms = ["linux/armv6"]
    args = {"image_name": "armhf-raspbian_buster",
    "from_builder":"balenalib/rpi-raspbian:buster",
    "extra_installs":"python2 python3"}
    output = ["./linux/armhf"]
}

target "arm64" {
    dockerfile = "BuildLib.Dockerfile"
    platforms = ["linux/arm64/v8"]
    args = {"image_name": "arm64-ubuntu_2004",
    "from_builder":"arm64v8/ubuntu:20.04",
    "extra_installs":"python-is-python3"}
    output = ["./linux/arm64"]
    
}
