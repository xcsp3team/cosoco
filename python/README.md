# cosoco

[cosoco](https://github.com/xcsp3team/cosoco) is a compact constraint solver,
written in C++, that reads [XCSP3](https://xcsp.org) instances.

This package ships nothing but the solver executable, built for the platform
of the wheel. It contains no binding: cosoco is meant to be run as a separate
process.

```bash
pip install cosoco
cosoco instance.xml -model=2
```

```python
import subprocess
from cosoco_bin import PATH

subprocess.run([PATH, "instance.xml", "-model=2"])
```

cosoco expects the instance first, before any option.

Modeling in Python is the job of [PyCSP3](https://pycsp.org), which drives
cosoco for you once it is installed:

```bash
pip install pycsp3[cosoco]
```
