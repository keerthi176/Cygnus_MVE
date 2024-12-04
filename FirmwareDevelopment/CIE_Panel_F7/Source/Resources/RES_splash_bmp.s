
								AREA    	|Splash_Data|, DATA, READONLY
								
								EXPORT 	cygnus_bitmap_x
								EXPORT	cygnus_bitmap_y	
								EXPORT	cygnus_bitmap_w	
								EXPORT	cygnus_bitmap_h	
								EXPORT	cygnus_bitmap_i [DATA]
									
								EXPORT 	smartnet_bitmap_x		
								EXPORT 	smartnet_bitmap_y		
								EXPORT 	smartnet_bitmap_w		
								EXPORT 	smartnet_bitmap_h		
								EXPORT 	smartnet_bitmap_i [DATA]
									
								EXPORT 	sitenet_bitmap_x		
								EXPORT 	sitenet_bitmap_y		
								EXPORT 	sitenet_bitmap_w		
								EXPORT 	sitenet_bitmap_h		
								EXPORT 	sitenet_bitmap_i [DATA]									
									
								ALIGN	


									
cygnus_bitmap_x			DCD		130
cygnus_bitmap_y			DCD		178
cygnus_bitmap_w			DCD		540
cygnus_bitmap_h			DCD		118	

cygnus_bitmap_i
								INCBIN  	RES_cygnus24.raw
							
								ALIGN
	
smartnet_bitmap_x			DCD		80
smartnet_bitmap_y			DCD		238
smartnet_bitmap_w			DCD		640
smartnet_bitmap_h			DCD		124	
	
smartnet_bitmap_i
								INCBIN 	RES_smartnet24.raw

								ALIGN
	
sitenet_bitmap_x			DCD		42
sitenet_bitmap_y			DCD		255
sitenet_bitmap_w			DCD		716
sitenet_bitmap_h			DCD		96	

sitenet_bitmap_i
								INCBIN 	RES_sitenet.raw

								END
									



			  