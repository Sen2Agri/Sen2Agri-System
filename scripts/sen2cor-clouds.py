#!/usr/bin/env python
import lxml
from lxml import etree
import os
import psycopg2
from psycopg2.sql import SQL, Literal

ids = []

with psycopg2.connect(dbname="sen4cap") as conn:
    with conn.cursor() as cursor:
        query = SQL(
            """
            select id, name, full_path
            from product
            where product_type_id = 1 and satellite_id = 1 and created_timestamp between '2020-03-01' and '2020-07-01'
            """
        )
        cursor.execute(query)
        for (id, name, full_path) in cursor:
            mtd_path = os.path.join(full_path, "MTD_MSIL2A.xml")
            tree = etree.parse(mtd_path)
            ns = "{https://psd-14.sentinel2.eo.esa.int/PSD/User_Product_Level-2A.xsd}"
            quality_indicators_info = tree.find(ns + "Quality_Indicators_Info")
            cloud_coverage_assessment = float(
                quality_indicators_info.findtext("Cloud_Coverage_Assessment")
            )
            image_content_qi = quality_indicators_info.find("Image_Content_QI")
            nodata_pixel_percentage = float(
                image_content_qi.findtext("NODATA_PIXEL_PERCENTAGE")
            )
            saturated_defective_pixel_percentage = float(
                image_content_qi.findtext("SATURATED_DEFECTIVE_PIXEL_PERCENTAGE")
            )
            dark_features_percentage = float(
                image_content_qi.findtext("DARK_FEATURES_PERCENTAGE")
            )
            cloud_shadow_percentage = float(
                image_content_qi.findtext("CLOUD_SHADOW_PERCENTAGE")
            )
            snow_ice_percentage = float(
                image_content_qi.findtext("SNOW_ICE_PERCENTAGE")
            )

            valid_pixels_percentage = (
                (100 - nodata_pixel_percentage)
                / 100.0
                * (
                    100
                    - cloud_coverage_assessment
                    - saturated_defective_pixel_percentage
                    - dark_features_percentage
                    - cloud_shadow_percentage
                    - snow_ice_percentage
                )
            )

            if valid_pixels_percentage < 5:
                print(
                    "{} {} {}".format(
                        name, valid_pixels_percentage, nodata_pixel_percentage
                    )
                )
                ids.append(id)

    query = SQL("delete from product where id in {};")
    query = query.format(Literal(tuple(ids)))
    print(query.as_string(conn))
