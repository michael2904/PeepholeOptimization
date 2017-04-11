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

/* iload x 1       iload x        iload x
 * ldc 0   2       ldc 1          ldc 2
 * imul    1       imul           imul
 * ------>        ------>        ------>
 * ldc 0   1       iload x        iload x
 *                                dup
 *                                iadd
 */

int simplify_multiplication_right_inverse(CODE **c)
{ int x,k;
	if (is_ldc_int(*c,&k) && 
			is_iload(next(*c),&x) && 
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

/* DONE
 * iload x   +1
 * ldc 1     +1
 * idiv      -1
 * ------>
 * iload x   +1
 */

int simplify_division_by_one_inverse(CODE **c)
{ int x,k;
	if (is_ldc_int(*c,&k) && 
			is_iload(next(*c),&x) && 
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
 * goto L1
 * ...
 * L1:
 * goto L2
 * ...
 * L2:
 * ...
 * goto L1
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
		return (replace_modified(c,1,makeCODEgoto(l2,NULL)));
	}
	return 0;
}

/* DONE
  (ldc || iload || aload)        +1
  dup           +1
  aload         +1
  swap           0
  putfield Interpretor/state Ljava/lang/String;  -2
  pop           -1
-------->
  aload         +1
  (ldc || iload || aload)    +1
  putfield Interpretor/state Ljava/lang/String;  -2
*/
int simplify_swap_put_field(CODE **c){
	int x, k;
	char *s1;
	char *s2;
	if(is_ldc_string(*c, &s1) &&
		is_dup(next(*c)) &&
		is_aload(next(next(*c)), &x) &&
		is_swap(next(next(next(*c)))) &&
		is_putfield(next(next(next(next(*c)))), &s2) &&
		is_pop(next(next(next(next(next(*c))))))){
			return replace(c, 6, makeCODEaload(x, makeCODEldc_string(s1, makeCODEputfield(s2, NULL))));
	} else if(is_ldc_int(*c, &k) &&
		is_dup(next(*c)) &&
		is_aload(next(next(*c)), &x) &&
		is_swap(next(next(next(*c)))) &&
		is_putfield(next(next(next(next(*c)))), &s2) &&
		is_pop(next(next(next(next(next(*c))))))){
			return replace(c, 6, makeCODEaload(x, makeCODEldc_int(k, makeCODEputfield(s2, NULL))));
	} else if(is_iload(*c, &k) &&
		is_dup(next(*c)) &&
		is_aload(next(next(*c)), &x) &&
		is_swap(next(next(next(*c)))) &&
		is_putfield(next(next(next(next(*c)))), &s2) &&
		is_pop(next(next(next(next(next(*c))))))){
			return replace(c, 6, makeCODEaload(x, makeCODEiload(k, makeCODEputfield(s2, NULL))));
	} else if(is_aload(*c, &k) &&
		is_dup(next(*c)) &&
		is_aload(next(next(*c)), &x) &&
		is_swap(next(next(next(*c)))) &&
		is_putfield(next(next(next(next(*c)))), &s2) &&
		is_pop(next(next(next(next(next(*c))))))){
			return replace(c, 6, makeCODEaload(x, makeCODEaload(k, makeCODEputfield(s2, NULL))));
	}
	return 0;
}

/* CHECK
  dup           +1
  aload         +1
  swap           0
  putfield Interpretor/state Ljava/lang/String;  -2
  pop           -1
-------->
  aload         +1
  swap
  putfield Interpretor/state Ljava/lang/String;  -2
*/
int simplify_put_field(CODE **c){
	int x;
	char *s;
	if(	is_dup(*c) &&
		is_aload(next(*c), &x) &&
		is_swap(next(next(*c))) &&
		is_putfield(next(next(next(*c))), &s) &&
		is_pop(next(next(next(next(*c)))))){
			return replace(c, 5, makeCODEaload(x, makeCODEswap(makeCODEputfield(s, NULL))));
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




/* DONE
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
int simplify_goto_consec_label(CODE **c)
{ int l1,l2;
	CODE *dest;
	if (is_goto(*c,&l1) || is_if(c,&l1)){
		dest = destination(l1);
		while(dest != NULL){
			if (is_label(next(dest),&l2)){
				dest = next(dest);
			}else{
				break;
			}
		}
		if(l1>l2){
			droplabel(l1);
			copylabel(l2);
			if(is_goto(*c,&l1)){
				return (replace_modified(c,1,makeCODEgoto(l2,NULL)));
			}else if(is_ifeq(*c,&l1)){
				return (replace_modified(c,1,makeCODEifeq(l2,NULL)));
			}else if(is_ifne(*c,&l1)){
				return (replace_modified(c,1,makeCODEifne(l2,NULL)));
			}else if(is_if_acmpeq(*c,&l1)){
				return (replace_modified(c,1,makeCODEif_acmpeq(l2,NULL)));
			}else if(is_if_acmpne(*c,&l1)){
				return (replace_modified(c,1,makeCODEif_acmpne(l2,NULL)));
			}else if(is_ifnull(*c,&l1)){
				return (replace_modified(c,1,makeCODEifnull(l2,NULL)));
			}else if(is_ifnonnull(*c,&l1)){
				return (replace_modified(c,1,makeCODEifnonnull(l2,NULL)));
			}else if(is_if_icmpeq(*c,&l1)){
				return (replace_modified(c,1,makeCODEif_icmpeq(l2,NULL)));
			}else if(is_if_icmpgt(*c,&l1)){
				return (replace_modified(c,1,makeCODEif_icmpgt(l2,NULL)));
			}else if(is_if_icmplt(*c,&l1)){
				return (replace_modified(c,1,makeCODEif_icmplt(l2,NULL)));
			}else if(is_if_icmple(*c,&l1)){
				return (replace_modified(c,1,makeCODEif_icmple(l2,NULL)));
			}else if(is_if_icmpge(*c,&l1)){
				return (replace_modified(c,1,makeCODEif_icmpge(l2,NULL)));
			}else if(is_if_icmpne(*c,&l1)){
				return (replace_modified(c,1,makeCODEif_icmpne(l2,NULL)));
			}
		}
	}
	return 0;
}

/* DONE
 * L1:        dead
 * ----->
 *
 */
int simplify_dead_label(CODE **c)
{ int l1;
	if (is_label(*c,&l1) && deadlabel(l1)) {
		return kill_line(c);
	}
	return 0;
}







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

 /*
 * if_icmpne l1       -2
 * iconst_k2  k2==0   +1  1/2
 * goto l2
 * l1:
 * iconst_k3  k3==1   +1  2/2
 * l2:
 * ifeq l3            -1
 * --------->
 * if_icmpeq l3
 */
 int simplify_generic_comp_not_eq(CODE **c)
{ int l1,l11,l2,l22,l3,k2,k3;
	if (is_if_icmpne(*c,&l1) &&
		(is_ldc_int(next(*c),&k2) && k2 == 0) &&
		is_goto(next(next(*c)),&l2) &&
		(is_label(next(next(next(*c))),&l11) && l1 == l11 && uniquelabel(l1)) &&
		(is_ldc_int(next(next(next(next(*c)))),&k3) && k3 == 1)&&
		(is_label(next(next(next(next(next(*c))))),&l22) && l2 == l22 && uniquelabel(l2)) &&
		is_ifeq(next(next(next(next(next(next(*c)))))),&l3)
		){
		return replace(c,7,makeCODEif_icmpeq(l3,NULL));
	}
	return 0;
}

/*
 * if_icmpeq l1
 * iconst_k2
 * goto l2
 * l1:
 * iconst_k3
 * l2:
 * ifeq l3
 * --------->
 * if_icmpne l3 
*/
 int simplify_generic_comp_eq(CODE **c)
{ int l1,l11,l2,l22,l3,k2,k3;
	if (is_if_icmpeq(*c,&l1) &&
		(is_ldc_int(next(*c),&k2) && k2 == 0) &&
		is_goto(next(next(*c)),&l2) &&
		(is_label(next(next(next(*c))),&l11) && l1 == l11 && uniquelabel(l1)) &&
		(is_ldc_int(next(next(next(next(*c)))),&k3) && k3 == 1)&&
		(is_label(next(next(next(next(next(*c))))),&l22) && l2 == l22 && uniquelabel(l2)) &&
		is_ifeq(next(next(next(next(next(next(*c)))))),&l3)
		){
		return replace(c,7,makeCODEif_icmpne(l3,NULL));
	}
	return 0;
}

/*
 * if_icmplt l1
 * iconst_k2  //k2 == 0
 * goto l2
 * l1:
 * iconst_k3  // k3 == 1
 * l2:
 * ifeq l3  if(k == 0) goto l3
 * --------->
 * if_icmpge l3 
*/
 int simplify_generic_comp_lt(CODE **c)
{ int l1,l11,l2,l22,l3,k2,k3;
	if (is_if_icmplt(*c,&l1) &&
		(is_ldc_int(next(*c),&k2) && k2 == 0) &&
		is_goto(next(next(*c)),&l2) &&
		(is_label(next(next(next(*c))),&l11) && l1 == l11 && uniquelabel(l1)) &&
		(is_ldc_int(next(next(next(next(*c)))),&k3) && k3 == 1)&&
		(is_label(next(next(next(next(next(*c))))),&l22) && l2 == l22 && uniquelabel(l2)) &&
		is_ifeq(next(next(next(next(next(next(*c)))))),&l3)
		){
		return replace(c,7,makeCODEif_icmpge(l3,NULL));
	}
	return 0;
}

/*
 * if_icmple l1
 * iconst_k2  //k2 == 0
 * goto l2
 * l1:
 * iconst_k3  // k3 == 1
 * l2:
 * ifeq l3  if(k == 0) goto l3
 * --------->
 * if_icmpgt l3 
*/
 int simplify_generic_comp_le(CODE **c)
{ int l1,l11,l2,l22,l3,k2,k3;
	if (is_if_icmple(*c,&l1) &&
		(is_ldc_int(next(*c),&k2) && k2 == 0) &&
		is_goto(next(next(*c)),&l2) &&
		(is_label(next(next(next(*c))),&l11) && l1 == l11 && uniquelabel(l1)) &&
		(is_ldc_int(next(next(next(next(*c)))),&k3) && k3 == 1)&&
		(is_label(next(next(next(next(next(*c))))),&l22) && l2 == l22 && uniquelabel(l2)) &&
		is_ifeq(next(next(next(next(next(next(*c)))))),&l3)
		){
		return replace(c,7,makeCODEif_icmpgt(l3,NULL));
	}
	return 0;
}

/*
 * if_icmpgt l1
 * iconst_k2  //k2 == 0
 * goto l2
 * l1:
 * iconst_k3  // k3 == 1
 * l2:
 * ifeq l3  if(k == 0) goto l3
 * --------->
 * if_icmple l3 
*/
 int simplify_generic_comp_gt(CODE **c)
{ int l1,l11,l2,l22,l3,k2,k3;
	if (is_if_icmple(*c,&l1) &&
		(is_ldc_int(next(*c),&k2) && k2 == 0) &&
		is_goto(next(next(*c)),&l2) &&
		(is_label(next(next(next(*c))),&l11) && l1 == l11 && uniquelabel(l1)) &&
		(is_ldc_int(next(next(next(next(*c)))),&k3) && k3 == 1)&&
		(is_label(next(next(next(next(next(*c))))),&l22) && l2 == l22 && uniquelabel(l2)) &&
		is_ifeq(next(next(next(next(next(next(*c)))))),&l3)
		){
		return replace(c,7,makeCODEif_icmple(l3,NULL));
	}
	return 0;
}

/*
 * if_icmpge l1
 * iconst_k2  //k2 == 0
 * goto l2
 * l1:
 * iconst_k3  // k3 == 1
 * l2:
 * ifeq l3  if(k == 0) goto l3
 * --------->
 * if_icmplt l3 
*/
 int simplify_generic_comp_ge(CODE **c)
{ int l1,l11,l2,l22,l3,k2,k3;
	if (is_if_icmple(*c,&l1) &&
		(is_ldc_int(next(*c),&k2) && k2 == 0) &&
		is_goto(next(next(*c)),&l2) &&
		(is_label(next(next(next(*c))),&l11) && l1 == l11 && uniquelabel(l1)) &&
		(is_ldc_int(next(next(next(next(*c)))),&k3) && k3 == 1)&&
		(is_label(next(next(next(next(next(*c))))),&l22) && l2 == l22 && uniquelabel(l2)) &&
		is_ifeq(next(next(next(next(next(next(*c)))))),&l3)
		){
		return replace(c,7,makeCODEif_icmplt(l3,NULL));
	}
	return 0;
}

/* THIS IS ME GOING CRAZY
  if_icmpge l1
  iconst_0
  goto l2
  l1:   (unique)
  iconst_1
  l3:
  l2: (unique)
  dup
  ifne l4
  pop
  aload_1
  ldc "use"
  iconst_0
  invokevirtual java/lang/String/indexOf(Ljava/lang/String;I)I
  iconst_0
  if_icmpge l5
  iconst_0
  goto l4
  l5:
  iconst_1
  l6:
  l4:
  ifeq l7
------------->
  if_icmpge true
  iconst_0
  l3:
  dup
  ifne l4
  pop
  aload_1
  ldc "use"
  iconst_0
  invokevirtual java/lang/String/indexOf(Ljava/lang/String;I)I
  iconst_0
  if_icmptlt l7
  goto true
  l4:
  ifeq else_0
  true:
*/

 int simplify_generic_OR_comp_ge(CODE **c)
{ int l1,l11,l2,l22,l3,l4,l44,l5,l55,l6,l66,l7,k2,k3;
	int x1, x2, x3, x4, x5;
	char *s1, *s2;
	if (is_if_icmpge(*c,&l1) &&
		(is_ldc_int(next(*c),&k2) && k2 == 0) &&
		(is_goto(next(next(*c)),&l2)) &&
		(is_label(next(next(next(*c))),&l11) && l1==l11) &&
		(is_ldc_int(next(next(next(next(*c)))),&k3) && k3 == 1) &&
		(is_label(next(next(next(next(next(*c))))),&l22)) && l2 == l22 &&
		(is_label(next(next(next(next(next(next(*c)))))),&l3) /*&& uniquelabel(l2)*/) &&
		(is_dup(next(next(next(next(next(next(next(*c))))))))) && 
		(is_ifne(next(next(next(next(next(next(next(next(*c)))))))), &l4)) && 
		(is_pop(next(next(next(next(next(next(next(next(next(*c))))))))))) && 
		(is_aload(next(next(next(next(next(next(next(next(next(next(*c)))))))))), &x1)) && 
		(is_ldc_string(next(next(next(next(next(next(next(next(next(next(*c)))))))))), &s1)) && 
		(is_ldc_int(next(next(next(next(next(next(next(next(next(next(next(*c))))))))))), &x2)) &&
		(is_invokevirtual(next(next(next(next(next(next(next(next(next(next(next(next(*c)))))))))))), &s2))  && 
		(is_ldc_int(next(next(next(next(next(next(next(next(next(next(next(next(next(*c))))))))))))), &x3)) &&
		(is_if_icmpge(next(next(next(next(next(next(next(next(next(next(next(next(next(next(*c)))))))))))))), &l5)) && 
		(is_ldc_int(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(*c))))))))))))))), &x4)) && x4==0 &&
		(is_goto(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(*c)))))))))))))))), &l6)) &&
		(is_label(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(*c))))))))))))))))), &l55)) && l55 == l5 &&
		(is_ldc_int(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(*c)))))))))))))))))), &x5)) && x5==1 &&
		(is_label(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(*c))))))))))))))))))), &l66)) && l66==l6 &&
		(is_label(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(*c)))))))))))))))))))), &l44)) && l44==l4 &&
		(is_ifeq(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(next(*c))))))))))))))))))))), &l7))
		){
		int nextL = next_label();
		char *nextLabel = (char*)malloc(10);
		sprintf(nextLabel, "label%d", nextL);
		return replace_modified(c,23,
			makeCODEif_icmplt((int)&(*nextLabel), 
			makeCODEldc_int(k2, 
			makeCODElabel(l3, 
			makeCODEdup(
			makeCODEifne(l4,
			makeCODEpop(
			makeCODEaload(x1,
			makeCODEldc_string(s1,
			makeCODEldc_int(x2,
			makeCODEinvokevirtual(s2,
			makeCODEldc_int(x3,
			makeCODEif_icmplt(l7,
			makeCODEgoto((int)&(*nextLabel),
			makeCODElabel(l4,
			makeCODEifeq(l1,
			makeCODElabel((int)&(*nextLabel), NULL)))))))))))))))));
	}
	return 0;
}

/*
  if_icmpge true
  iconst_0
  l3:
  dup
  ifne l4
  pop
  aload_1
  ldc "use"
  iconst_0
  invokevirtual java/lang/String/indexOf(Ljava/lang/String;I)I
  iconst_0
  if_icmptlt l7
  goto true
  l4:
  ifeq else_0
  true:
*/

 int simplify_nop_after_return(CODE **c){
 	if (is_return(*c) &&
 		is_nop(next(*c))){
		return replace(c,1,makeCODEreturn(NULL));
	} else if (is_ireturn(*c) &&
 		is_nop(next(*c))){
		return replace(c,1,makeCODEireturn(NULL));
	} else if (is_areturn(*c) &&
 		is_nop(next(*c))){
		return replace(c,1,makeCODEareturn(NULL));
	}
	return 0;
}

/*
  aload_0
  getfield Decoder/uti Llib/JoosBitwise;
  aload_0
  getfield Decoder/uti Llib/JoosBitwise;
-------->
  aload_0
  getfield Decoder/uti Llib/JoosBitwise;
  dup
*/
int simplify_redundant_get_field(CODE **c){
	int x,y;
	char *f1, *f2;
	if(is_aload(*c, &x) &&
		is_getfield(next(*c), &f1) &&
		is_aload(next(next(*c)), &y) && x==y &&
		is_getfield(next(next(next(*c))), &f2) && (strcmp(f1,f2) == 0)
	){
		return replace(c,4,makeCODEaload(x, makeCODEgetfield(f1, makeCODEdup(NULL))));
	}
	return 0;
}



void init_patterns(void) {
	ADD_PATTERN(simplify_goto_consec_label);
	// ADD_PATTERN(simplify_nop_after_return);
	// ADD_PATTERN(simplify_dead_label);
	ADD_PATTERN(simplify_multiplication_right);
	ADD_PATTERN(simplify_multiplication_right_inverse);
	ADD_PATTERN(simplify_division_by_one);
	ADD_PATTERN(simplify_division_by_one_inverse);
	ADD_PATTERN(simplify_astore);
	ADD_PATTERN(simplify_istore);
	ADD_PATTERN(simplify_aload_astore);
	ADD_PATTERN(simplify_iload_istore);
	ADD_PATTERN(positive_increment);
	ADD_PATTERN(negative_increment);
	ADD_PATTERN(simplify_goto_goto);
	ADD_PATTERN(simplify_swap_put_field);
	ADD_PATTERN(simplify_put_field);
	ADD_PATTERN(simplify_object_assignment);
	ADD_PATTERN(simplify_generic_comp_not_eq);
	ADD_PATTERN(simplify_generic_comp_eq);
	ADD_PATTERN(simplify_generic_comp_lt);
	ADD_PATTERN(simplify_generic_comp_le);
	ADD_PATTERN(simplify_generic_comp_gt);
	ADD_PATTERN(simplify_generic_comp_ge);
	ADD_PATTERN(simplify_redundant_get_field);
	// ADD_PATTERN(simplify_generic_OR_comp_ge);
}
