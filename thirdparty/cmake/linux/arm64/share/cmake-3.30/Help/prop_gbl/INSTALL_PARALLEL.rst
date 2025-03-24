INSTALL_PARALLEL
----------------

.. versionadded:: 3.30

Enables parallel installation option for the Ninja generator.

When this property is ``ON``, ``install/local`` targets have the
console pool disabled, allowing them to run concurrently.

This property also provides the target ``install/parallel``, which has an
explicit dependency on the ``install/local`` target for each subdirectory,
recursing down the project.

Setting this property has no affect on the behavior of ``cmake --install``.
The install must be invoked by building the ``install/parallel`` target
directly.

Calls to :command:`install(CODE)` or :command:`install(SCRIPT)` might depend
on actions performed by an earlier :command:`install` command in a different
directory such as files installed or variable settings. If the project has
such order-dependent installation logic, parallel installation should be
not be enabled, in order to prevent possible race conditions.
