configure_python() {
    nap_root=$1
    thirdparty="$nap_root/thirdparty"
    python="$thirdparty/python/bin/python3"
    if [ ! -e "$python" ]; then
        thirdparty="$nap_root/../thirdparty"
        if [ "$(uname)" = "Linux" ]; then
            case "$(arch)" in
            "x86_64")
                python="$thirdparty/python/linux/x86_64/bin/python3"
                ;;
            "aarch64")
                python="$thirdparty/python/linux/arm64/bin/python3"
                ;;
            *)
                python="$thirdparty/python/linux/armhf/bin/python3"
                ;;
            esac
        else
            python="$thirdparty/python/macos/x86_64/bin/python3"
        fi
    fi
    unset PYTHONHOME
    unset PYTHONPATH
}
