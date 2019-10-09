all:
	@echo "       ╔═╗┬ ┬┬┌┬┐┌─┐┬ ┬       "           
	@echo "       ╚═╗││││ │ │  ├─┤       " 
	@echo "       ╚═╝└┴┘┴ ┴ └─┘┴ ┴       " 
	@echo "             and              "
	@echo "           ╦ ╦┬┬╦ ╦           "                                                         
	@echo "           ║║║││║ ║           "
	@echo "           ╚╩╝┴┴╚═╝           " 	 
	@echo "  Compile For Switch and WiiU "
	@echo
	@echo -n "           [Y / N] " && read ans && [ $${ans:-N} = y ]
	make -j8 -f Makefile.switch
	make -j8 -f Makefile.wiiu

switch:
	@echo "       ╔═╗┬ ┬┬┌┬┐┌─┐┬ ┬       "                 
	@echo "       ╚═╗││││ │ │  ├─┤       "      
	@echo "       ╚═╝└┴┘┴ ┴ └─┘┴ ┴       "      
	@echo  
	@echo "      Compile For Switch      "
	@echo
	@echo -n "           [Y / N] " && read ans && [ $${ans:-N} = y ]
	make -j8 -f Makefile.switch

wiiu:
	@echo "           ╦ ╦┬┬╦ ╦           "                                                         
	@echo "           ║║║││║ ║           "
	@echo "           ╚╩╝┴┴╚═╝           " 
	@echo  
	@echo "       Compile For WiiU       "
	@echo
	@echo -n "        [Y / N] " && read ans && [ $${ans:-N} = y ]
	make -j8 -f Makefile.wiiu

clean: 
	make -f Makefile.switch clean
	make -f Makefile.wiiu clean