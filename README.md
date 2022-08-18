# SHtack
The russian roulette for nerds


Heres how it works:

Theres a stack of sh commands. Client sends a(2 currently) command(s) in, they get 1(one) command out.
The client is then supposed to execute the command on their machine

Routes:

/push: push(POST) a command to the stack. Accepts a post body, key of command, value of whatever your command is. [footnote 1]

/pop: GET a command from the stack.


Inputs are sanitized server-side, sudo, su and && are not allowed


Footnote 1{push redirects}: On a successful push, the response will have code 303. Then, it will give you a new path to follow. If the path is /push/[SOMENUMBER], you will be doing a push again. If it is /pop/[SOMENUMBER], you can proceed to pop.

