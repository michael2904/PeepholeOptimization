COMP520_CodeOptimization

Michael Abdallah-Minciotti
Clovis Rigout


Possible optimizations:
	- L1 ... L2 ---> L2 and change all L1 to L2
	- optimize comparison with 0


invokevirtual BacktrackSolver/getVal(II)I
iconst_0
if_icmpne true_3
iconst_0
goto stop_4
true_3:
iconst_1
stop_4:
ifeq else_1

------>

invokevirtual BacktrackSolver/getVal(II)I
ifeq else_1



invokevirtual BacktrackSolver/getVal(II)I
iconst_0
if_icmpeq true_9
iconst_0
goto stop_10
true_9:
iconst_1
stop_10:
ifeq stop_8

------->
invokevirtual BacktrackSolver/getVal(II)I
ifne stop_8