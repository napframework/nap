variable "x86_64_id" {
    default = "x86_64-ubuntu_2004"
}

target "x86_64" {
    platforms = ["linux/amd64"]
    dockerfile = "Dockerfile.create-${x86_64_id}"
    args = {"image_name": "${x86_64_id}"}
    tags = ["nap-${x86_64_id}:latest"]
}

variable "armhf_id" {
    default = "armhf-raspbian_buster"
}

target "armhf" {
    platforms = ["linux/armv6"]
    dockerfile = "Dockerfile.create-${armhf_id}"
    args = {"image_name": "${armhf_id}"}
    tags = ["nap-${armhf_id}:latest"]
}

variable "arm64_id" {
    default = "arm64-ubuntu_2004"
}

target "arm64" {
    platforms = ["linux/arm64/v8"]
    dockerfile = "Dockerfile.create-${arm64_id}"
    args = {"image_name": "${arm64_id}"}
    tags = ["nap-${arm64_id}:latest"]
}
