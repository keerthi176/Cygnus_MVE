	   /********************************************************************************
		********************************************************************************
		**
		** VARIABLE NAMING CONVENTION
		**
		********************************************************************************
		********************************************************************************


		[ Dave Pemberton! ]
		
		The naming convention I use which makes it easy to tell any variable or data-type
		just by looking at it.
		This lowers the chance of data manipulation errors.
		- eg: INTerger maths on a CHARacter.
		Expecting a value above 0xFF,
		which can never happen & would wait for that condition forever...


		[ int8_t ]		(8 Bits)
		-
		cName			: signed CHAR
		pcName			: pointer to a signed CHAR
		acName			: array of signed CHAR'S
		pacName			: pointer to an array of signed CHAR'S
		apcName			: array of pointers to signed CHAR'S
		papcName		: pointer to an array of pointers to signed CHAR'S

		[ uint8_t ]		(8 Bits)
		-
		ucName			: unsigned CHAR
		pucName			: pointer to an unsigned CHAR
		aucName			: array of unsigned CHAR'S
		paucName		: pointer to an array of unsigned CHAR'S
		apucName		: array of pointers to unsigned CHAR'S
		papucName		: pointer to an array of pointers to unsigned CHAR'S




		[ int16_t]		(16 Bits)
		-
		sName			: signed SHORT
		psName			: pointer to a signed SHORT
		asName			: array of signed SHORTS
		pasName			: pointer to an array of signed SHORTS
		apsName			: array of pointers to signed SHORTS
		papsName		: pointer to an array of pointers to signed SHORTS
		-
		[ uint16_t]		(16 Bits)
		-
		usName			: unsigned SHORT
		pusName			: pointer to an unsigned SHORTS
		ausName			: array of unsigned SHORTS
		pausName		: pointer to an array of unsigned SHORTS
		apusName		: array of pointers to unsigned SHORTS
		papusName		: pointer to an array of pointers to unsigned SHORTS




		[ int32_t ]		(32 bits)
		iName			: signed INT
		piName			: pointer to a signed INT
		aiName			: array of signed INT'S
		paiName			: pointer to an array of signed INT'S
		apiName			: array of pointers to signed INT'S
		papiName		: pointer to an array of pointers to signed INT'S
		:
		[ uint32_t ]	(32 bits)
		-
		uiName			: unsigned INT
		puiName			: pointer to an unsigned INT
		auiName			: array of unsigned INT'S
		pauiName		: pointer to an array of unsigned INT'S
		apuiName		: array of pointers to unsigned INT'S
		papuiName		: pointer to an array of pointers to unsigned INT'S




		[ long32_t ]	(32 bits)
		-
		lName			: signed LONG
		plName			: pointer to a signed LONG
		alName			: array of signed LONGS
		palName			: pointer to an array of signed LONGS
		aplName			: array of pointers to signed LONGS 
		paplName		: pointer to an array of pointers to signed LONGS
		:
		[ ulong32_t ]	(32 bits)
		-
		ulName			: unsigned LONG
		pulName			: pointer to an unsigned LONG
		aulName			: array of unsigned LONGS
		paulName		: pointer to an array of unsigned LONGS
		apulName		: array of pointers to unsigned LONGS
		papulName		: pointer to an array of pointers to unsigned LONGS




		[ long64_t ]	(64 bits)
		-
		llName			: signed LONGLONG
		pllName			: pointer to a signed LONGLONG
		allName			: array of signed LONGLONGS
		pallName		: pointer to an array of signed LONGLONGS
		apllName		: array of pointers to signed LONGLONGS
		papllName		: pointer to an array of pointers to signed LONGLONGS
		:
		[ ulong64_t ]	(64 bits)
		-
		ullName			: unsigned LONGLONG
		pullName		: pointer to an unsigned LONGLONG
		aullName		: array of unsigned LONGLONGS
		paullName		: pointer to an array of unsigned LONGLONGS
		apullName		: array of pointers to unsigned LONGLONGS
		papullName		: pointer to an array of pointers to unsigned LONGLONGS




		[ FLOAT ]
		-
		fName			: FLOAT
		pfName			: pointer to a FLOAT
		afName			: array of FLOATS
		pafName			: pointer to an array of FLOATS
		apfName			: array of pointers to FLOATS
		papfName		: pointer to an array of pointers to FLOATS




		[ DOUBLE ]
		-
		dName			: DOUBLE
		pdName			: pointer to a DOUBLE
		adName			: array of DOUBLES
		padName			: pointer to an array of DOUBLES
		apdName			: array of pointers to DOUBLES
		papdName		: pointer to an array of pointers to DOUBLES




		[ STRUCTURE ]
		-
		dxName			: STRUCTURE definition
		xName			: STRUCTURE
		axName			: array of STRUCTURES
		pxName			: pointer to a STRUCTURE
		paxName			: pointer to an array of STRUCTURES
		apxName			: array of pointer to an array of STRUCTURES
		papxName		: pointer to an array of pointers to STRUCTURES




		[ UNION ]
		-
		duxName			: union definition
		uxName			: UNION
		auxName			: array of UNIONS
		puxName			: pointer to a UNION
		pauxName		: pointer to an array of UNION
		apuxName		: array of pointer to an array of UNIONS
		papuxName		: pointer to an array of pointers to UNIONS




		[ CLASS ]
		-
		CName			: CLASS definition
		obName			: OBJECT instance
		pobName			: pointer to an OBJECT
		apobName		: array of pointers to OBJECTS
		paobName		: pointer to an array of OBJECTS
		papobName		: pointer to an array of pointers to OBJECTS




		[ CLASS MEMBER ]
		-
		m_Name			: MEMBER defintion/prototype
		pmName			: pointer to a MEMBER
		amName			: array of MEMBERS
		pamName			: pointer to an array of MEMBERS
		apmName			: array of pointers to MEMBERS
		papmName		: pointer to an array of pointers to MEMBERS




		[ FUNCTION ]
		-
		f_Name			: FUNCTION defintion/prototype
		f_pfName		: pointer to a FUNCTION
		af_Name			: array of FUNCTIONS
		paf_Name		: pointer to an array of FUNCTIONS
		apf_Name		: array of pointers to FUNCTIONS
		papf_Name		: pointer to an array of pointers to FUNCTIONS




		[ STRING ]
		-
		szName			: ASCII string terminated by zero
		aszName			: array of ASCII strings terminated by zero
		pszName			: pointer to an ASCII string terminated by zero
		paszName		: pointer to an array of ASCII strings terminated by zero
	



		[ HANDLE ] 		: HANDLE definition
		-
		hName 			: handle                            
		ahName  		: array of handles                  
		pahName			: pointer to an array of handles    




		[ VECTOR ]
		-
		vxName			: VECTOR definition
		avxName			: array of VECTORS
		pvxName			: pointer to a VECTOR
		pavxName		: pointer to an array of VECTORS
		apvxName		: array of pointer to an array of VECTORS
		papvxName		: pointer to an array of pointers to VECTORS




		[ LIST ]
		-
		lsName			: LIST definition
		alsName			: array of LIST
		plsName			: pointer to a LIST
		plsqName		: pointer to an array of LISTS
		aplsName		: array of pointer to an array of LISTS
		paplsName		: pointer to an array of pointers to LISTS




		[ DEQUE ]
		-
		dqName			: DEQUE definition
		adqName			: array of DEQUES
		pdqName			: pointer to a DEQUE
		padqName		: pointer to an array of DEQUES
		apdqName		: array of pointer to an array of DEQUES
		papdqName		: pointer to an array of pointers to DEQUES




		[ MAP ]
		-
		mpName			: MAP definition
		ampName			: array of MAPS
		pmpName			: pointer to a MAP
		pampName		: pointer to an array of MAPS
		apmpName		: array of pointer to an array of MAPS
		papmpName		: pointer to an array of pointers to MAPS




		lpv				: prefixed to any of the above == long pointer to a void of this type
		vo	 			: prefixed to any of the above == VOLATILE




		[ ENUM ]		: Definition
		ENUM_NAME		: Enumeration

		[ ENU ]			: Enumeration Entry
		ENU_NAME		: Enumeration




		[ HDF_ ]		: #define - (definition)
		[ MAC_ ]		: #define - (MACRO)




		[ Dave Pemberton! ]


		********************************************************************************
		********************************************************************************
		**
		** VARIABLE NAMING CONVENTION
		**
		********************************************************************************
		*******************************************************************************/

