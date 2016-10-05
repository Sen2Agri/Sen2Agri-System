#!/bin/sh

rpm -Uvh --force ../rpm_binaries/*.rpm

for file in migrations/*.sql
do
    cat "$file" | sudo -u postgres psql sen2agri
done