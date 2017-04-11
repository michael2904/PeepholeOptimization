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

/* iload x 1       iload x        iload x
 * ldc 0   2       ldc 1          ldc 2
 * imul    1       imul           imul
 * ------>        ------>        ------>
 * ldc 0   1       iload x        iload x
 *                                dup
 *                                iadd
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

/* DONE
 * iload x   +1
 * ldc 1     +1
 * idiv      -1
 * ------>
 * iload x   +1
 */

/* ----- Duplicate for inverse order ldc then iload */

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

/* dup        +1
 * astore x   -1
 * pop        -1
 * -------->
 * astore x   -1
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

/* DONE
 * dup       +1
 * istore x  -1
 * pop       -1
 * -------->
 * istore x  -1
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

/* DONE
 * aload x  +1
 * astore x -1
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

/* DONE
 * iload x   +1
 * istore x  -1
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

/* DOESN'T WORK ******* LOOK AGAIN LATER
 * dup     +1
 * ifeq L1 -1
 * pop     -1 1/2
 * ...
 * L1:
 * pop     -1 2/2
 * -------->
 * ifeq L1
 * ...
 * L1:
 *
 */

/* iload x              +1
 * ldc k   (0<=k<=127)  +1
 * iadd                 -1
 * istore x             -1
 * --------->
 * iinc x k             no change
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

/* DONE
 * iload x             +1
 * ldc k   (0<=k<=127) +1
 * isub                -1
 * istore x            -1
 * --------->
 * iinc x -k           no change
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
 *  && replace(&(next(destination(l1))),1,NULL) to take out useless label
 */
int simplify_goto_goto(CODE **c)
{ int l1,l2;
	if (is_goto(*c,&l1) &&
		is_goto(next(destination(l1)),&l2) && l1>l2) {
		droplabel(l1);
		copylabel(l2);
		return (replace(c,1,makeCODEgoto(l2,NULL)));
	}
	return 0;
}

/* DONE
 * iconst_k1  k1==0   +1
 * if_icmpne l1       -2
 * iconst_k2  k2==0   +1  1/2
 * goto l2
 * l1:
 * iconst_k3  k3==1   +1  2/2
 * l2:
 * ifeq l3            -1
 * --------->
 * ifeq l3            -1
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


/* DONE
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

/* DONE
  ldc "init"    +1
  dup           +1
  aload         +1
  swap           0
  putfield Interpretor/state Ljava/lang/String;  -2
  pop           -1
-------->
  aload         +1
  ldc "init"    +1
  putfield Interpretor/state Ljava/lang/String;  -2
*/
int simplify_string_decl(CODE **c){
	int x;
	char *s1;
	char *s2;
	if(is_ldc_string(*c, &s1) &&
		is_dup(next(*c)) &&
		is_aload(next(next(*c)), &x) &&
		is_swap(next(next(next(*c)))) &&
		is_putfield(next(next(next(next(*c)))), &s2) &&
		is_pop(next(next(next(next(next(*c))))))){
			return replace(c, 6, makeCODEaload(x, makeCODEldc_string(s1, makeCODEputfield(s2, NULL))));
	}
	return 0;
}

/* DONE                                                 ~
  new Conversion (java class)                           conv
  dup                                                   conv conv
  invokenonvirtual Conversion/<init>()V                 conv
  dup                                                   conv conv
  aload_0                                               self conv conv
  swap                                                  conv self conv
  putfield Decoder/con LConversion;                     conv
  pop                                                   ~
 ------->
  aload_0                                               self
  new Conversion                                        conv self
  dup                                                   conv conv self
  invokenonvirtual Conversion/<init>()V                 conv self
  putfield Decoder/con LConversion;                     ~
*/
int simplify_object_assignment(CODE **c){
	char *o1, *v1, *f1;
	int x;
	if(is_new(*c, &o1) &&
		is_dup(next(*c)) &&
		is_invokenonvirtual(next(next(*c)), &v1) &&
		is_dup(next(next(next(*c)))) &&
		is_aload(next(next(next(next(*c)))), &x) &&
		is_swap(next(next(next(next(next(*c)))))) &&
		is_putfield(next(next(next(next(next(next(*c)))))), &f1) &&
		is_pop(next(next(next(next(next(next(next(*c))))))))){
			return replace(c, 8, makeCODEaload(x, makeCODEnew(o1, makeCODEdup(makeCODEinvokenonvirtual(v1, makeCODEputfield(f1, NULL))))));
	}
	return 0;
}




/* TOOO CHECK
 * goto/any branching L1
 * ...
 * goto/any branching L2
 * ...
 * L1:
 * L2:
 * --------->
 * goto/any branching L2
 * ...
 * goto/any branching L2
 * ...
 * L1:       (reference count reduced by 1)
 * L2:       (reference count increased by 1)  
 */




/* TOOO CHECK
 * iconst_k1  k1!=0   +1
 * if_icmpne l1       -2
 * iconst_k2  k2==0   +1  1/2
 * goto l2
 * l1:
 * iconst_k3  k3==1   +1  2/2
 * l2:
 * ifeq l3            -1
 * --------->
 * iconst_k1  k1!=0   +1
 * isub               -1
 * ifeq l3            -1
 * 
 * Also works with iloads instead of iconst
 * Also works with any kind of comparison
 * 		substract and use the comparison ifeq... with 0
 */

void init_patterns(void) {
	ADD_PATTERN(simplify_multiplication_right);
	ADD_PATTERN(simplify_division_by_one);
	ADD_PATTERN(simplify_astore);
	ADD_PATTERN(simplify_istore);
	ADD_PATTERN(simplify_aload_astore);
	ADD_PATTERN(simplify_iload_istore);
	ADD_PATTERN(positive_increment);
	ADD_PATTERN(negative_increment);
	ADD_PATTERN(simplify_goto_goto);
	ADD_PATTERN(simplify_comp_not_eq_zero);
	ADD_PATTERN(simplify_comp_eq_zero);
	ADD_PATTERN(simplify_string_decl);
	ADD_PATTERN(simplify_object_assignment);
}
