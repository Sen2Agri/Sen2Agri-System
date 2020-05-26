FROM centos:centos7

LABEL name="sen2agri-services" \
      maintainer="Laurentiu Nicola <laurentiu.nicola@c-s.ro>"

RUN yum -y install yum-utils epel-release https://download.postgresql.org/pub/repos/yum/reporpms/EL-7-x86_64/pgdg-redhat-repo-latest.noarch.rpm && \
    yum-config-manager --disable pgdg95 && \
    yum -y update && \
    yum -y install postgresql12 python-psycopg2 R-devel libcurl-devel openssl-devel libxml2-devel && \
    echo -e 'packages <- c("e1071", "caret", "dplyr", "gsubfn", "ranger", "readr", "smotefamily", "caTools", "tidyverse", "data.table")\n\
             diff <- setdiff(packages, rownames(installed.packages()))\n\
             if (length(diff) > 0) {\n\
                 install.packages(diff, repos = c(CRAN = "https://cran.rstudio.com"))\n\
             }' | Rscript -

COPY *.rpm /

# install gdal explicitly to prevent otb from pulling in libgeotiff-1.4
RUN yum install -y gdal /otb-*.rpm /sen2agri-*.rpm /gdal-*.rpm && \
    rm /*.rpm && \
    ldconfig
