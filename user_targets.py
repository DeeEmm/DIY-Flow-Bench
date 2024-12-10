Import("env")

# Single action/command per 1 target
env.AddCustomTarget("Sys Env", None, 'python -c "import os; print(os.environ)"')

# Multiple actions
env.AddCustomTarget(
    name="pioenv",
    dependencies=None,
    actions=[
        "pio --version",
        "python --version"
    ],
    title="Core Env",
    description="Show PlatformIO Core and Python versions"
)

env.AddCustomTarget(
    name="Build_GUI",
    dependencies=None,
    actions=[
        "pio --clean"
    ],
    title="Build GUI Only",
    description="This is a test"
)
