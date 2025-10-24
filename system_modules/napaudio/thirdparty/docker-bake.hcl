target "x86_64" {
    dockerfile = "GetDependencies.Dockerfile"
    platforms = ["linux/amd64"]
    args = {"from_builder":"ubuntu:24.04",
        TARGET_NAME  = "x86_64",
        "extra_installs":"python-is-python3"}
    output = ["./"]
}

target "armhf" {
    dockerfile = "GetDependencies.Dockerfile"
    platforms = ["linux/armv6"]
    args = {"from_builder" : "balenalib/rpi-raspbian:buster",
        TARGET_NAME  = "armhf",
        "extra_installs":"python2 python3"}
    output = ["./"]
}

target "arm64" {
    dockerfile = "GetDependencies.Dockerfile"
    platforms = ["linux/arm64/v8"]
    args = { "from_builder" : "arm64v8/ubuntu:24.04",
        TARGET_NAME  = "arm64",
        "extra_installs":"python-is-python3"}
    output = ["./"]
}
