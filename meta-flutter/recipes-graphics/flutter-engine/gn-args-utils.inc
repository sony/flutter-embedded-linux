def get_engine_artifact_dir(d):
    gn_args = d.getVar("GN_ARGS")

    artifact_dir = 'out/'
    if 'debug' in gn_args:
        artifact_dir += 'linux_debug_unopt_arm64'
    elif 'profile' in gn_args:
        artifact_dir += 'linux_profile_arm64'
    else:
        artifact_dir += 'linux_release_arm64'

    return artifact_dir