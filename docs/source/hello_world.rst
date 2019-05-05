Hello World!
============

A *Hello World* in foolysh would look something like this:

.. code-block:: python

    from foolysh import app


    class MyApp(app.App):
        def __init__(self):
            super(MyApp, self).__init__('MyApp')


    MyApp().run()


This code would just open an empty window with the title "MyApp".

