import subprocess

def before_build(source, target, env):
    latest_version = subprocess.check_output(["git", "describe", "--tags"]).strip().decode("utf-8")
    print(f"Latest version: {latest_version}")  # Debug print
    version_file = "version.h"
    with open(version_file, "r") as file:
        lines = file.readlines()
    with open(version_file, "w") as file:
        for line in lines:
            if line.startswith("#define SOFTWARE_VERSION"):
                file.write(f'#define SOFTWARE_VERSION "{latest_version}"\n')
            else:
                file.write(line)
