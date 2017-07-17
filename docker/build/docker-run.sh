docker run -it \
       -v $(realpath ../..):/sen2agri \
       -u $(id -u):$(id -g) \
       sen2agri-build /bin/bash build-sen2agri.sh
