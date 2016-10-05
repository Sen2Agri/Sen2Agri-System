#!/bin/sh

systemctl stop sen2agri-orchestrator sen2agri-http-listener sen2agri-sentinel-downloader sen2agri-landsat-downloader sen2agri-monitor-agent

rpm -Uvh --force ../rpm_binaries/*.rpm

for file in migrations/*.sql
do
    cat "$file" | sudo -u postgres psql sen2agri
done

systemctl start sen2agri-orchestrator sen2agri-http-listener sen2agri-sentinel-downloader sen2agri-landsat-downloader sen2agri-monitor-agent
