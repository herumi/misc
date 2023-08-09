define i32 @invModPre_org(i32* %py, i32 %x, i32 %m) {
start:
  %b1 = icmp eq i32 %x, 0
  br i1 %b1, label %exit_lp, label %entry

entry:
  %k = phi i32 [ %k7, %lp ], [ 0, %start ]
  %s = phi i32 [ %s6, %lp ], [ 1, %start ]
  %r = phi i32 [ %r5, %lp ], [ 0, %start ]
  %v = phi i32 [ %v5, %lp ], [ %x, %start ]
  %u = phi i32 [ %u5, %lp ], [ %m, %start ]
  %lsb1 = and i32 %u, 1
  %b2 = icmp eq i32 %lsb1, 0
  br i1 %b2, label %even_u, label %odd_u

even_u:
  %u1 = ashr i32 %u, 1
  %s1 = shl i32 %s, 1
  br label %lp

odd_u:
  %v1 = and i32 %v, 1
  %b3 = icmp eq i32 %v1, 0
  br i1 %b3, label %even_v, label %odd_v

even_v:
  %v2 = ashr i32 %v, 1
  %r2 = shl i32 %r, 1
  br label %lp

odd_v:
  %b4 = icmp sgt i32 %u, %v
  br i1 %b4, label %u_gt_v, label %u_le_v

u_gt_v:
  %u2 = sub nsw i32 %u, %v
  %u3 = ashr i32 %u2, 1
  %r3 = add nsw i32 %s, %r
  %s3 = shl i32 %s, 1
  br label %lp

u_le_v:
  %v3 = sub nsw i32 %v, %u
  %v4 = ashr i32 %v3, 1
  %s4 = add nsw i32 %s, %r
  %r4 = shl i32 %r, 1
  br label %lp

lp:
  %u5 = phi i32 [ %u1, %even_u ], [ %u, %even_v ], [ %u3, %u_gt_v ], [ %u, %u_le_v ]
  %v5 = phi i32 [ %v, %even_u ], [ %v2, %even_v ], [ %v, %u_gt_v ], [ %v4, %u_le_v ]
  %r5 = phi i32 [ %r, %even_u ], [ %r2, %even_v ], [ %r3, %u_gt_v ], [ %r4, %u_le_v ]
  %s6 = phi i32 [ %s1, %even_u ], [ %s, %even_v ], [ %s3, %u_gt_v ], [ %s4, %u_le_v ]
  %k7 = add nuw nsw i32 %k, 1
  %b5 = icmp eq i32 %v5, 0
  br i1 %b5, label %exit_lp, label %entry

exit_lp:
  %r6 = phi i32 [ 0, %start ], [ %r5, %lp ]
  %k8 = phi i32 [ 0, %start ], [ %k7, %lp ]
  %b6 = icmp sgt i32 %r6, %m
  %y8 = select i1 %b6, i32 %m, i32 0
  %r7 = sub i32 %m, %r6
  %r8 = add i32 %r7, %y8
  store i32 %r8, i32* %py
  ret i32 %k8
}
