FROM hepsw/cvmfs-cms

RUN yum install -y \
    glibc-headers \
    glibc-devel \
