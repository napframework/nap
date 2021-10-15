variable "x86_64_id" {
    default = "x86_64-ubuntu_2004"
}

target "x86_64" {
    platforms = ["linux/amd64"]
    args = {
        "image_name": "nap-${x86_64_id}",
        "arch": "x86_64"
    }
    output = ["./nap-${x86_64_id}"]
}

variable "armhf_id" {
    default = "armhf-raspbian_buster"
}

target "armhf" {
    platforms = ["linux/armv6"]
    args = {
        "image_name": "nap-${armhf_id}",
        "arch": "armhf"
    }
    output = ["./nap-${armhf_id}"]
}

variable "arm64_id" {
    default = "arm64-ubuntu_2004"
}

target "arm64" {
    platforms = ["linux/arm64/v8"]
    args = {
        "image_name": "nap-${arm64_id}",
        "arch": "arm64"
    }
    output = ["./nap-${arm64_id}"]
}
