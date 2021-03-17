000000a4 <add256_u32>:
        .local i64
      a9: 20 00        	local.get	0
      ab: 20 02        	local.get	2
      ad: 35 02 00     	i64.load32_u	0
      b0: 20 01        	local.get	1
      b2: 35 02 00     	i64.load32_u	0
      b5: 7c           	i64.add 
      b6: 22 03        	local.tee	3
      b8: 3e 02 00     	i64.store32	0
      bb: 20 00        	local.get	0
      bd: 20 03        	local.get	3
      bf: 42 20        	i64.const	32
      c1: 88           	i64.shr_u
      c2: 20 01        	local.get	1
      c4: 35 02 04     	i64.load32_u	4
      c7: 7c           	i64.add 
      c8: 20 02        	local.get	2
      ca: 35 02 04     	i64.load32_u	4
      cd: 7c           	i64.add 
      ce: 22 03        	local.tee	3
      d0: 3e 02 04     	i64.store32	4
      d3: 20 00        	local.get	0
      d5: 20 03        	local.get	3
      d7: 42 20        	i64.const	32
      d9: 88           	i64.shr_u
      da: 20 01        	local.get	1
      dc: 35 02 08     	i64.load32_u	8
      df: 7c           	i64.add 
      e0: 20 02        	local.get	2
      e2: 35 02 08     	i64.load32_u	8
      e5: 7c           	i64.add 
      e6: 22 03        	local.tee	3
      e8: 3e 02 08     	i64.store32	8
      eb: 20 00        	local.get	0
      ed: 20 03        	local.get	3
      ef: 42 20        	i64.const	32
      f1: 88           	i64.shr_u
      f2: 20 01        	local.get	1
      f4: 35 02 0c     	i64.load32_u	12
      f7: 7c           	i64.add 
      f8: 20 02        	local.get	2
      fa: 35 02 0c     	i64.load32_u	12
      fd: 7c           	i64.add 
      fe: 22 03        	local.tee	3
     100: 3e 02 0c     	i64.store32	12
     103: 20 00        	local.get	0
     105: 20 03        	local.get	3
     107: 42 20        	i64.const	32
     109: 88           	i64.shr_u
     10a: 20 01        	local.get	1
     10c: 35 02 10     	i64.load32_u	16
     10f: 7c           	i64.add 
     110: 20 02        	local.get	2
     112: 35 02 10     	i64.load32_u	16
     115: 7c           	i64.add 
     116: 22 03        	local.tee	3
     118: 3e 02 10     	i64.store32	16
     11b: 20 00        	local.get	0
     11d: 20 03        	local.get	3
     11f: 42 20        	i64.const	32
     121: 88           	i64.shr_u
     122: 20 01        	local.get	1
     124: 35 02 14     	i64.load32_u	20
     127: 7c           	i64.add 
     128: 20 02        	local.get	2
     12a: 35 02 14     	i64.load32_u	20
     12d: 7c           	i64.add 
     12e: 22 03        	local.tee	3
     130: 3e 02 14     	i64.store32	20
     133: 20 00        	local.get	0
     135: 20 03        	local.get	3
     137: 42 20        	i64.const	32
     139: 88           	i64.shr_u
     13a: 20 01        	local.get	1
     13c: 35 02 18     	i64.load32_u	24
     13f: 7c           	i64.add 
     140: 20 02        	local.get	2
     142: 35 02 18     	i64.load32_u	24
     145: 7c           	i64.add 
     146: 22 03        	local.tee	3
     148: 3e 02 18     	i64.store32	24
     14b: 20 00        	local.get	0
     14d: 20 02        	local.get	2
     14f: 28 02 1c     	i32.load	28
     152: 20 01        	local.get	1
     154: 28 02 1c     	i32.load	28
     157: 6a           	i32.add 
     158: 20 03        	local.get	3
     15a: 42 20        	i64.const	32
     15c: 88           	i64.shr_u
     15d: a7           	i32.wrap_i64
     15e: 6a           	i32.add 
     15f: 36 02 1c     	i32.store	28
     162: 0b           	end
