# A collections of make functions

ifndef FUNCTIONS_MK
FUNCTIONS_MK=


# space
noop=
SPACE:=$(noop) $(noop)
COMMA:=,

# Emulation of DIR
DIR=$(notdir $(subst /.,,$(wildcard $1/*/.)))

# Make does not offer a recursive wildcard function, so here's one:
rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))
# Examples
#FOLDERS:=$(sort $(dir $(call rwildcard,$(CURDIR)/,*)))
#$(foreach FOLDER,$(FOLDERS),$(info $(FOLDER)))
#C_FILES:=$(sort $(dir $(call rwildcard,$(CURDIR)/,*.c)))
#$(foreach C_FILE,$(C_FILES),$(info $(C_FILE)))

# get the real path of the folder  or not if it does not exist
REALPATH_OR_NOT=$(if $(realpath $(dir $1)),$(realpath $(dir $1))/$(notdir $1),$1)

# cheap lower/upper case transform (only for ASCII)
lc = $(subst A,a,$(subst B,b,$(subst C,c,$(subst D,d,$(subst E,e,$(subst F,f,$(subst G,g,$(subst H,h,$(subst I,i,$(subst J,j,$(subst K,k,$(subst L,l,$(subst M,m,$(subst N,n,$(subst O,o,$(subst P,p,$(subst Q,q,$(subst R,r,$(subst S,s,$(subst T,t,$(subst U,u,$(subst V,v,$(subst W,w,$(subst X,x,$(subst Y,y,$(subst Z,z,$1))))))))))))))))))))))))))
up = $(subst a,A,$(subst b,B,$(subst c,C,$(subst d,D,$(subst e,E,$(subst f,F,$(subst g,G,$(subst h,H,$(subst i,I,$(subst j,J,$(subst k,K,$(subst l,L,$(subst m,M,$(subst n,N,$(subst o,O,$(subst p,P,$(subst q,Q,$(subst r,R,$(subst s,S,$(subst t,T,$(subst u,U,$(subst v,V,$(subst w,W,$(subst x,X,$(subst y,Y,$(subst z,Z,$1))))))))))))))))))))))))))

endif