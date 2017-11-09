#!/bin/sh

mysql -u root -p -e "create database ems_data" ;
mysql -u root -p ems_data < cs744.sql ;