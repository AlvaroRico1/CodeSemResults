.. _cmd-end:

end - end a block of commands
=============================

Synopsis
--------

.. synopsis::

    begin
        [COMMANDS ...] 
    end

.. synopsis::

    function NAME [OPTIONS]; COMMANDS ...; end
    if CONDITION; COMMANDS_TRUE ...; [else; COMMANDS_FALSE ...;] end
    switch VALUE; [case [WILDCARD ...]; [COMMANDS ...]; ...] end
    while CONDITION; COMMANDS ...; end
    for VARNAME in [VALUES ...]; COMMANDS ...; end

Description
-----------

The **end** keyword ends a block of commands started by one of the following commands:

- :ref:`begin <cmd-begin>` to start a block of commands
- :ref:`function <cmd-function>` to define a function
- :ref:`if <cmd-if>`, :ref:`switch <cmd-switch>` to conditionally execute commands
- :ref:`while <cmd-while>`, :ref:`for <cmd-for>` to perform commands multiple times

The **end** keyword does not change the current exit status.
Instead, the status after it will be the status returned by the most recent command.
