00000001 <add256_extInt>:
        .local i64, i64, i64, i64, i64, i64, i64, i64
       6: 20 02        	local.get	2
       8: 41 18        	i32.const	24
       a: 6a           	i32.add 
       b: 29 03 00     	i64.load	0
       e: 21 03        	local.set	3
      10: 20 01        	local.get	1
      12: 41 18        	i32.const	24
      14: 6a           	i32.add 
      15: 29 03 00     	i64.load	0
      18: 21 04        	local.set	4
      1a: 20 02        	local.get	2
      1c: 41 10        	i32.const	16
      1e: 6a           	i32.add 
      1f: 29 03 00     	i64.load	0
      22: 21 05        	local.set	5
      24: 20 01        	local.get	1
      26: 41 10        	i32.const	16
      28: 6a           	i32.add 
      29: 29 03 00     	i64.load	0
      2c: 21 06        	local.set	6
      2e: 20 02        	local.get	2
      30: 41 08        	i32.const	8
      32: 6a           	i32.add 
      33: 29 03 00     	i64.load	0
      36: 21 07        	local.set	7
      38: 20 01        	local.get	1
      3a: 41 08        	i32.const	8
      3c: 6a           	i32.add 
      3d: 29 03 00     	i64.load	0
      40: 21 08        	local.set	8
      42: 20 00        	local.get	0
      44: 20 02        	local.get	2
      46: 29 03 00     	i64.load	0
      49: 22 09        	local.tee	9
      4b: 20 01        	local.get	1
      4d: 29 03 00     	i64.load	0
      50: 7c           	i64.add 
      51: 22 0a        	local.tee	10
      53: 37 03 00     	i64.store	0
      56: 20 00        	local.get	0
      58: 20 07        	local.get	7
      5a: 20 08        	local.get	8
      5c: 7c           	i64.add 
      5d: 20 0a        	local.get	10
      5f: 20 09        	local.get	9
      61: 54           	i64.lt_u
      62: 22 01        	local.tee	1
      64: ad           	i64.extend_i32_u
      65: 7c           	i64.add 
      66: 22 08        	local.tee	8
      68: 37 03 08     	i64.store	8
      6b: 20 00        	local.get	0
      6d: 20 05        	local.get	5
      6f: 20 06        	local.get	6
      71: 7c           	i64.add 
      72: 22 06        	local.tee	6
      74: 20 01        	local.get	1
      76: 20 08        	local.get	8
      78: 20 07        	local.get	7
      7a: 54           	i64.lt_u
      7b: 20 08        	local.get	8
      7d: 20 07        	local.get	7
      7f: 51           	i64.eq  
      80: 1b           	f32.select
      81: ad           	i64.extend_i32_u
      82: 7c           	i64.add 
      83: 22 07        	local.tee	7
      85: 37 03 10     	i64.store	16
      88: 20 00        	local.get	0
      8a: 41 18        	i32.const	24
      8c: 6a           	i32.add 
      8d: 20 03        	local.get	3
      8f: 20 04        	local.get	4
      91: 7c           	i64.add 
      92: 20 06        	local.get	6
      94: 20 05        	local.get	5
      96: 54           	i64.lt_u
      97: ad           	i64.extend_i32_u
      98: 7c           	i64.add 
      99: 20 07        	local.get	7
      9b: 20 06        	local.get	6
      9d: 54           	i64.lt_u
      9e: ad           	i64.extend_i32_u
      9f: 7c           	i64.add 
      a0: 37 03 00     	i64.store	0
      a3: 0b           	end
