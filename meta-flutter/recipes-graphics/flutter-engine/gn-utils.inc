def get_out_dir(d):
    gn_args = d.getVar("GN_ARGS")

    if 'debug' in gn_args:
        out_dir = 'linux_debug_unopt_arm64'
    elif 'profile' in gn_args:
        out_dir = 'linux_profile_arm64'
    else:
        out_dir = 'linux_release_arm64'

    return out_dir