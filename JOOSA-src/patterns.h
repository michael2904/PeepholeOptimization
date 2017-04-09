/*
 * JOOS is Copyright (C) 1997 Laurie Hendren & Michael I. Schwartzbach
 *
 * Reproduction of all or part of this software is permitted for
 * educational or research use on condition that this copyright notice is
 * included in any copy. This software comes with no warranty of any
 * kind. In no event will the authors be liable for any damages resulting from
 * use of this software.
 *
 * email: hendren@cs.mcgill.ca, mis@brics.dk
 */

/* iload x        iload x        iload x
 * ldc 0          ldc 1          ldc 2
 * imul           imul           imul
 * ------>        ------>        ------>
 * ldc 0          iload x        iload x
 *                               dup
 *                               iadd
 */

int simplify_multiplication_right(CODE **c)
{ int x,k;
	if (is_iload(*c,&x) && 
			is_ldc_int(next(*c),&k) && 
			is_imul(next(next(*c)))) {
		 if (k==0) return replace(c,3,makeCODEldc_int(0,NULL));
		 else if (k==1) return replace(c,3,makeCODEiload(x,NULL));
		 else if (k==2) return replace(c,3,makeCODEiload(x,makeCODEdup(makeCODEiadd(NULL))));
		 return 0;
	}
	return 0;
}

/* TOOO CHECK
 * iload x
 * ldc 1
 * idiv
 * ------>
 * ldc x
 */

int simplify_division_by_one(CODE **c)
{ int x,k;
	if (is_iload(*c,&x) && 
			is_ldc_int(next(*c),&k) && 
			is_idiv(next(next(*c)))) {
		if (k==1) return replace(c,3,makeCODEiload(x,NULL));
		return 0;
	}
	return 0;
}

/* dup
 * astore x
 * pop
 * -------->
 * astore x
 */
int simplify_astore(CODE **c)
{ int x;
	if (is_dup(*c) &&
			is_astore(next(*c),&x) &&
			is_pop(next(next(*c)))) {
		 return replace(c,3,makeCODEastore(x,NULL));
	}
	return 0;
}

/* dup
 * istore x
 * pop
 * -------->
 * istore x
 */
int simplify_istore(CODE **c)
{ int x;
	if (is_dup(*c) &&
			is_istore(next(*c),&x) &&
			is_pop(next(next(*c)))) {
		 return replace(c,3,makeCODEistore(x,NULL));
	}
	return 0;
}

/* TOOO CHECK
 * astore x
 * aload x
 * -------->
 */
int simplify_astore_aload(CODE **c)
{ int x,y;
	if (is_astore(*c,&x) &&
			is_aload(next(*c),&y)) {
		if(x == y) return replace(c,2,NULL);
		return 0;
	}
	return 0;
}

/* TOOO CHECK
 * istore x
 * iload x
 * -------->
 */
int simplify_istore_iload(CODE **c)
{ int x,y;
	if (is_istore(*c,&x) &&
			is_iload(next(*c),&y)) {
		if(x == y) return replace(c,2,NULL);
		return 0;
	}
	return 0;
}

/* TOOO CHECK
 * aload x
 * astore x
 * -------->
 */
int simplify_aload_astore(CODE **c)
{ int x,y;
	if (is_aload(*c,&x) &&
			is_astore(next(*c),&y)) {
		if(x == y) return replace(c,2,NULL);
		return 0;
	}
	return 0;
}

/* TOOO CHECK
 * iload x
 * istore x
 * -------->
 */
int simplify_iload_istore(CODE **c)
{ int x,y;
	if (is_iload(*c,&x) &&
			is_istore(next(*c),&y)) {
		if(x == y) return replace(c,2,NULL);
		return 0;
	}
	return 0;
}

/* TOOO CHECK
 * dup
 * ifeq x
 * pop
 * -------->
 * ifeq x
 */
int simplify_ifeq(CODE **c)
{ int x;
	if (is_dup(*c) &&
			is_ifeq(next(*c),&x) &&
			is_pop(next(next(*c)))) {
		 return replace(c,3,makeCODEifeq(x,NULL));
	}
	return 0;
}

/* TOOO CHECK
 * dup
 * ifne x
 * pop
 * -------->
 * ifne x
 */
int simplify_ifne(CODE **c)
{ int x;
	if (is_dup(*c) &&
			is_ifne(next(*c),&x) &&
			is_pop(next(next(*c)))) {
		 return replace(c,3,makeCODEifne(x,NULL));
	}
	return 0;
}

/* iload x
 * ldc k   (0<=k<=127)
 * iadd
 * istore x
 * --------->
 * iinc x k
 */ 
int positive_increment(CODE **c)
{ int x,y,k;
	if (is_iload(*c,&x) &&
			is_ldc_int(next(*c),&k) &&
			is_iadd(next(next(*c))) &&
			is_istore(next(next(next(*c))),&y) &&
			x==y && 0<=k && k<=127) {
		 return replace(c,4,makeCODEiinc(x,k,NULL));
	}
	return 0;
}

/* TOOO CHECK
 * iload x
 * ldc k   (0<=k<=127)
 * isub
 * istore x
 * --------->
 * iinc x -k
 */ 
int negative_increment(CODE **c)
{ int x,y,k;
	if (is_iload(*c,&x) &&
			is_ldc_int(next(*c),&k) &&
			is_isub(next(next(*c))) &&
			is_istore(next(next(next(*c))),&y) &&
			x==y && 0<=k && k<=127) {
		 return replace(c,4,makeCODEiinc(x,-k,NULL));
	}
	return 0;
}

/* goto L1
 * ...
 * L1:
 * goto L2
 * ...
 * L2:
 * --------->
 * goto L2
 * ...
 * L1:    (reference count reduced by 1)
 * goto L2
 * ...
 * L2:    (reference count increased by 1)  
 */
int simplify_goto_goto(CODE **c)
{ int l1,l2;
	if (is_goto(*c,&l1) &&
		is_goto(next(destination(l1)),&l2) && l1>l2) {
		droplabel(l1);
		copylabel(l2);
		return replace(c,1,makeCODEgoto(l2,NULL));
	}
	return 0;
}

/* TOOO CHECK
 * iconst_k1
 * if_icmpne l1
 * iconst_k2
 * goto l2
 * l1:
 * iconst_k3
 * l2:
 * ifeq l3
 * --------->
 * ifeq l3 
 */
int simplify_comp_not_eq_zero(CODE **c)
{ int l1,l11,l2,l22,l3,k1,k2,k3;
	if ((is_ldc_int(*c,&k1) && k1 == 0) &&
		is_if_icmpne(next(*c),&l1) &&
		(is_ldc_int(next(next(*c)),&k2) && k2 == 0) &&
		is_goto(next(next(next(*c))),&l2) &&
		(is_label(next(next(next(next(*c)))),&l11) && l1 == l11) &&
		(is_ldc_int(next(next(next(next(next(*c))))),&k3) && k3 == 1)&&
		(is_label(next(next(next(next(next(next(*c)))))),&l22) && l2 == l22) &&
		is_ifeq(next(next(next(next(next(next(next(*c))))))),&l3)
		){
		return replace(c,8,makeCODEifeq(l3,NULL));
	}
	return 0;
}


/* TOOO CHECK
 * iconst_k1
 * if_icmpeq l1
 * iconst_k2
 * goto l2
 * l1:
 * iconst_k3
 * l2:
 * ifeq l3
 * --------->
 * ifne l3 
 */
int simplify_comp_eq_zero(CODE **c)
{ int l1,l11,l2,l22,l3,k1,k2,k3;
	if ((is_ldc_int(*c,&k1) && k1 == 0) &&
		is_if_icmpeq(next(*c),&l1) &&
		(is_ldc_int(next(next(*c)),&k2) && k2 == 0) &&
		is_goto(next(next(next(*c))),&l2) &&
		(is_label(next(next(next(next(*c)))),&l11) && l1 == l11) &&
		(is_ldc_int(next(next(next(next(next(*c))))),&k3) && k3 == 1)&&
		(is_label(next(next(next(next(next(next(*c)))))),&l22) && l2 == l22) &&
		is_ifeq(next(next(next(next(next(next(next(*c))))))),&l3)
		){
		return replace(c,8,makeCODEifne(l3,NULL));
	}
	return 0;
}


void init_patterns(void) {
	ADD_PATTERN(simplify_multiplication_right);
	ADD_PATTERN(simplify_division_by_one);
	ADD_PATTERN(simplify_astore);
	ADD_PATTERN(simplify_istore);
	ADD_PATTERN(simplify_astore_aload);
	ADD_PATTERN(simplify_istore_iload);
	ADD_PATTERN(simplify_aload_astore);
	ADD_PATTERN(simplify_iload_istore);	
	ADD_PATTERN(positive_increment);
	ADD_PATTERN(negative_increment);
	ADD_PATTERN(simplify_goto_goto);
	ADD_PATTERN(simplify_comp_not_eq_zero);
	ADD_PATTERN(simplify_comp_eq_zero);
	ADD_PATTERN(simplify_ifeq);
	ADD_PATTERN(simplify_ifne);
}
