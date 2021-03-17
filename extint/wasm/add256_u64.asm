000003bd <add256_u64>:
        .local i64, i64, i64, i64, i32
     3c4: 20 00        	local.get	0
     3c6: 20 01        	local.get	1
     3c8: 29 03 00     	i64.load	0
     3cb: 22 03        	local.tee	3
     3cd: 20 02        	local.get	2
     3cf: 29 03 00     	i64.load	0
     3d2: 7c           	i64.add 
     3d3: 22 04        	local.tee	4
     3d5: 37 03 00     	i64.store	0
     3d8: 20 00        	local.get	0
     3da: 20 02        	local.get	2
     3dc: 29 03 08     	i64.load	8
     3df: 22 05        	local.tee	5
     3e1: 20 05        	local.get	5
     3e3: 20 01        	local.get	1
     3e5: 29 03 08     	i64.load	8
     3e8: 22 06        	local.tee	6
     3ea: 20 04        	local.get	4
     3ec: 20 03        	local.get	3
     3ee: 54           	i64.lt_u
     3ef: ad           	i64.extend_i32_u
     3f0: 7c           	i64.add 
     3f1: 22 03        	local.tee	3
     3f3: 7c           	i64.add 
     3f4: 22 04        	local.tee	4
     3f6: 20 03        	local.get	3
     3f8: 20 06        	local.get	6
     3fa: 54           	i64.lt_u
     3fb: 22 07        	local.tee	7
     3fd: 1b           	f32.select
     3fe: 37 03 08     	i64.store	8
     401: 20 00        	local.get	0
     403: 20 02        	local.get	2
     405: 29 03 10     	i64.load	16
     408: 22 05        	local.tee	5
     40a: 20 05        	local.get	5
     40c: 20 01        	local.get	1
     40e: 29 03 10     	i64.load	16
     411: 22 06        	local.tee	6
     413: 20 07        	local.get	7
     415: 20 04        	local.get	4
     417: 20 03        	local.get	3
     419: 54           	i64.lt_u
     41a: 72           	i32.or  
     41b: ad           	i64.extend_i32_u
     41c: 7c           	i64.add 
     41d: 22 03        	local.tee	3
     41f: 7c           	i64.add 
     420: 22 04        	local.tee	4
     422: 20 03        	local.get	3
     424: 20 06        	local.get	6
     426: 54           	i64.lt_u
     427: 22 07        	local.tee	7
     429: 1b           	f32.select
     42a: 37 03 10     	i64.store	16
     42d: 20 00        	local.get	0
     42f: 42 00        	i64.const	0
     431: 20 01        	local.get	1
     433: 29 03 18     	i64.load	24
     436: 22 05        	local.tee	5
     438: 20 07        	local.get	7
     43a: 20 04        	local.get	4
     43c: 20 03        	local.get	3
     43e: 54           	i64.lt_u
     43f: 72           	i32.or  
     440: ad           	i64.extend_i32_u
     441: 7c           	i64.add 
     442: 22 03        	local.tee	3
     444: 20 03        	local.get	3
     446: 20 05        	local.get	5
     448: 54           	i64.lt_u
     449: 1b           	f32.select
     44a: 20 02        	local.get	2
     44c: 29 03 18     	i64.load	24
     44f: 7c           	i64.add 
     450: 37 03 18     	i64.store	24
     453: 0b           	end

