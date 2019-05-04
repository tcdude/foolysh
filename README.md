# foolysh

Simple 2D Rendering Engine based on PySDL2 with intended deployment on Android and potentially also iOS in the future.


This is a work in progress...  

you can install `foolysh` using pip: 

`pip install git+https://github.com/tcdude/foolysh.git`

see also the generated [docs](https://www.tizilogic.com/various/foolysh-engine)

##### Simple Example:

```python
from foolysh import app

class MyApp(app.App):
    def __init__(self):
        super(MyApp, self).__init__('MyApp')
        self.task_manager.add_task('quitter', self.quit, delay=5, 
                                   with_dt=False, blocking=False)
        

MyApp().run()
```

 


