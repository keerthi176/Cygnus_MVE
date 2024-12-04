
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
																		
									
								ALIGN	


									
cygnus_bitmap_x			DCD		130
cygnus_bitmap_y			DCD		78
cygnus_bitmap_w			DCD		540
cygnus_bitmap_h			DCD		118	

cygnus_bitmap_i
								INCBIN  	cygnus24.raw
							
								ALIGN
	
smartnet_bitmap_x			DCD		80
smartnet_bitmap_y			DCD		238
smartnet_bitmap_w			DCD		640
smartnet_bitmap_h			DCD		124	
	

smartnet_bitmap_i
								INCBIN 	smartnet24.raw
	
								END



			  