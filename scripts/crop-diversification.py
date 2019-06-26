#!/usr/bin/python

import argparse
import csv


def main():
    parser = argparse.ArgumentParser(description="Crop type diversification script")
    parser.add_argument('-i', '--input', help="classification result CSV")
    parser.add_argument('-l', '--lut', help="LUT CSV")
    parser.add_argument('-o', '--output', help="output CSV")

    args = parser.parse_args()

    input_csv = args.input
    input_lut = args.lut
    output_csv = args.output
    parcel_id_field = 'NewID'
    holding_id_field = 'HoldID'
    crop_divers_field_decl = 'CTnumDIV'
    crop_divers_field_pred_1 = 'CTnumDIV_pred_1'
    area_field = 'Area_meters'
    conf_threshold = 2.0

    aggDict = {}
    cat = {}
    cropdiv = {}
    debug = {}

    output_fields_int = ['nb_types_c', 'NewID', 'HoldID', 'CT_decl', 'CT_pred_1', 'CT_pred_2', 'Ori_crop', crop_divers_field_decl, crop_divers_field_pred_1, 'nb_parcels_nc', 'LC', 'S2pix', 'S1pix']
    output_fields_float = [area_field, 'area_eaa_c', 'area_tal_c', 'area_tempGrass_c', 'area_permGrass_c', 'area_llf_c', 'area_cwater_c', 'area_remAl_ex2_c', 'area_remAl_ex3_c', 'area_mainCrop_c', 'area_2mainCrop_c', 'area_nc', 'CT_conf_1', 'CT_conf_2']
    output_fields_str = []
    output_fields = ['NewID', 'Classif_r', 'CD_cat', 'CD_diagn', 'Area_meters']
    db_fields = ['nb_types_c', 'area_eaa_c', 'area_tal_c', 'area_tempGrass_c', 'area_permGrass_c', 'area_llf_c', 'area_cwater_c', 'area_remAl_ex2_c', 'area_remAl_ex3_c', 'area_mainCrop_c', 'area_2mainCrop_c', 'nb_parcels_nc', 'area_nc']
    agri_fields = ['HoldID', 'CD_cat', 'CD_diagn']

    # --------------------------------------------------------------------------------
    # Create the lists of CTnumDIV in the different categories: Eligible Agriculture Area (EAA), Arable Land (AL), Permanent Grassland (PGrass), Temporary Grassland (TGrass), Land Lying Fallow (Fallow) and Crop Under Water (Cwater)

    eaa_codes = set()
    tal_codes = set()
    permGrass_codes = set()
    tempGrass_codes = set()
    llf_codes = set()
    cwater_codes = set()

    with open(input_lut) as l_in:
        lut_in = csv.DictReader(l_in, delimiter=',')

        for row in lut_in:
            if (row.get('eaa') or row.get('EAA')) == '1':
                eaa_codes.add(str(row['ctnumdiv']))
            if (row.get('eaa') or row.get('AL')) == '1':
                tal_codes.add(str(row['ctnumdiv']))
            if (row.get('pgrass') or row.get('PGrass')) == '1':
                permGrass_codes.add(str(row['ctnumdiv']))
            if (row.get('tgrass') or row.get('TGrass')) == '1':
                tempGrass_codes.add(str(row['ctnumdiv']))
            if (row.get('fallow') or row.get('Fallow')) == '1':
                llf_codes.add(str(row['ctnumdiv']))
            if (row.get('cwater') or row.get('Cwater')) == '1':
                cwater_codes.add(str(row['ctnumdiv']))

    # --------------------------------------------------------------------------------
    # Import data file

    with open(input_csv) as f_in:
        csv_in = csv.DictReader(f_in, delimiter=',')

    # --------------------------------------------------------------------------------
    # Create column with results of the classification "Classif_r"

        for row in csv_in:
            agri = row[holding_id_field]
            fid = row[parcel_id_field]
            out = dict(row)
            if agri not in aggDict:
                aggDict[agri] = {}
                debug[agri] = {}
            if row['CT_pred_1'] != 'NA' and row['CT_pred_1'] != '':
                if row['CT_decl'] == row['CT_pred_1'] or row['CT_decl'] == row['CT_pred_2']:
                    out['Classif_r'] = 'Classified_conform'
                elif float(row['CT_conf_1']) >= conf_threshold:
                    out['Classif_r'] = 'Classified_not_conform_prediction_used'
                else:
                    out['Classif_r'] = 'Classified_not_conform'
            elif row['GeomValid'] == '0' or row['Duplic'] == '0' or row['Overlap'] == '1':
                out['Classif_r'] = 'Not_classified_geometry'
            elif row['LC'] == '' or row['LC'] == '0' or row['LC'] == '5' or row['LC'] == 'NA':
                out['Classif_r'] = 'Not_classified_land_cover'
            elif row['S2Pix'] == '' or int(row['S2Pix']) <= 2:
                out['Classif_r'] = 'Not_classified_minS2pix'
            elif row['S1Pix'] == '' or int(row['S1Pix']) == 0:
                out['Classif_r'] = 'Not_classified_noS1pix'
            else:
                out['Classif_r'] = 'Not_classified_undefined'
            aggDict[agri][fid] = out

    # --------------------------------------------------------------------------------
    # Summarized factors by holding

        for agri in aggDict:
            area_eaa_c = 0.
            area_tal_c = 0.
            area_tempGrass_c = 0.
            area_permGrass_c = 0.
            area_llf_c = 0.
            area_cwater_c = 0.
            area_remAl_ex2_c = 0.
            area_remAl_ex3_c = 0.
            types_c = set()
            nb_parcels_nc = 0.
            area_mainCrop_c = 0.
            area_2mainCrop_c = 0.
            area_nc = 0.
            areas_dict = {}

            for (fid, row) in aggDict[agri].items():
                if row['Classif_r'] == 'Classified_conform':
                    if row[crop_divers_field_decl] == '':
                        cType = str(row[crop_divers_field_decl])
                    else:
                        cType = str(int(row[crop_divers_field_decl]))
                elif row['Classif_r'] == 'Classified_not_conform_prediction_used':
                    if row[crop_divers_field_decl] == '':
                        cType = str(row[crop_divers_field_pred_1])
                    else:
                        cType = str(int(row[crop_divers_field_pred_1]))
                else:
                    if row[crop_divers_field_decl] == '':
                        cType = str(row[crop_divers_field_decl])
                    else:
                        cType = str(int(row[crop_divers_field_decl]))
                area = float(row[area_field])
                if row['Classif_r'] == 'Classified_conform' or row['Classif_r'] == 'Classified_not_conform_prediction_used':
                    if cType in eaa_codes:
                        area_eaa_c += area
                    if cType in tal_codes:
                        area_tal_c += area
                        types_c.add(cType)
                        areas_dict[cType] = area if cType not in areas_dict else area + areas_dict[cType]
                    if cType in tempGrass_codes:
                        area_tempGrass_c += area
                    if cType in permGrass_codes:
                        area_permGrass_c += area
                    if cType in llf_codes:
                        area_llf_c += area
                    if cType in cwater_codes:
                        area_cwater_c += area
                elif cType in eaa_codes:
                    area_nc += area
                    nb_parcels_nc += 1

            nb_types_c = len(types_c)
            area_remAl_ex2_c = area_tal_c - area_tempGrass_c - area_llf_c
            area_remAl_ex3_c = area_tal_c - area_tempGrass_c - area_cwater_c

            areas = list(areas_dict.values())
            areas.sort(reverse=True)

            area_mainCrop_c = 0. if len(areas) == 0 else areas[0]
            area_2mainCrop_c = 0. if len(areas) == 0 or len(areas) == 1 else areas[1]

    # --------------------------------------------------------------------------------
    # Define the crop diversification category "CD_cat"

            cat[agri] = 'Exemption_or_Category1_2_or_3'

            if area_nc == 0:

                if area_tal_c < 100000:
                    cat[agri] = 'Exemption1'

                elif area_tempGrass_c + area_llf_c > 0.75 * area_tal_c and area_remAl_ex2_c <= 300000:
                    cat[agri] = 'Exemption2'

                elif area_permGrass_c + area_tempGrass_c + area_cwater_c > 0.75 * area_eaa_c and area_remAl_ex3_c <= 300000:
                    cat[agri] = 'Exemption3'

                elif area_cwater_c == area_tal_c:
                    cat[agri] = 'Exemption4'

                elif area_tempGrass_c + area_llf_c > 0.75 * area_tal_c:
                    cat[agri] = 'Category3'

                elif area_tal_c > 100000 and area_tal_c <= 300000:
                    cat[agri] = 'Category1'

                elif area_tal_c > 300000:
                    cat[agri] = 'Category2'

            elif area_nc != 0:

                if area_tal_c + area_nc < 100000:
                    cat[agri] = 'Exemption1'

                elif area_tempGrass_c + area_llf_c > 0.75 * (area_tal_c + area_nc) and area_remAl_ex2_c + area_nc <= 300000:
                    cat[agri] = 'Exemption2'

                elif area_permGrass_c + area_tempGrass_c + area_cwater_c > 0.75 * (area_eaa_c + area_nc) and area_remAl_ex3_c + area_nc <= 300000:
                    cat[agri] = 'Exemption3'

                elif area_tal_c < 100000 or (area_tempGrass_c + area_llf_c > 0.75 * area_tal_c and area_remAl_ex2_c <= 300000) or (area_permGrass_c + area_tempGrass_c + area_cwater_c > 0.75 * area_eaa_c and area_remAl_ex3_c <= 300000) or (area_cwater_c == area_tal_c):

                    if area_tempGrass_c + area_llf_c > 0.75 * area_tal_c:

                        if area_tempGrass_c + area_llf_c > 0.75 * (area_tal_c + area_nc):
                            cat[agri] = 'Exemption_or_Category3'

                        elif area_tal_c + area_nc >= 100000 and area_tal_c + area_nc < 300000:
                            cat[agri] = 'Exemption_or_Category1_or_3'

                        elif area_tal_c >= 300000:
                            cat[agri] = 'Exemption_or_Category2_or_3'

                        elif area_tal_c + area_nc >= 300000:
                            cat[agri] = 'Exemption_or_Category1_2_or_3'

                    elif area_tempGrass_c + area_llf_c + area_nc <= 0.75 * (area_tal_c + area_nc):

                        if area_tal_c + area_nc >= 100000 and area_tal_c + area_nc < 300000:
                            cat[agri] = 'Exemption_or_Category1'

                        elif area_tal_c >= 300000:
                            cat[agri] = 'Exemption_or_Category2'

                        elif area_tal_c + area_nc >= 300000:
                            cat[agri] = 'Exemption_or_Category1_or_2'

                    elif area_tal_c + area_nc >= 100000 and area_tal_c + area_nc < 300000:
                        cat[agri] = 'Exemption_or_Category1_or_3'

                    elif area_tal_c >= 300000:
                        cat[agri] = 'Exemption_or_Category2_or_3'

                    elif area_tal_c + area_nc >= 300000:
                        cat[agri] = 'Exemption_or_Category1_2_or_3'

                elif area_tal_c > 100000 and ((area_tempGrass_c + area_llf_c + area_nc <= 0.75 * (area_tal_c + area_nc)) or (area_tempGrass_c + area_llf_c + area_nc > 0.75 * (area_tal_c + area_nc) and area_remAl_ex2_c > 300000) or (area_tempGrass_c + area_llf_c > 0.75 * (area_tal_c + area_nc) and area_remAl_ex2_c + area_nc > 300000) or (area_tempGrass_c + area_llf_c > 0.75 * area_tal_c and area_remAl_ex2_c > 300000)) and ((area_permGrass_c + area_tempGrass_c + area_cwater_c + area_nc <= 0.75 * (area_eaa_c + area_nc)) or (area_permGrass_c + area_tempGrass_c + area_cwater_c > 0.75 * area_eaa_c and area_remAl_ex3_c > 300000) or (area_permGrass_c + area_tempGrass_c + area_cwater_c + area_nc > 0.75 * (area_eaa_c + area_nc) and area_remAl_ex3_c > 300000)) and (area_cwater_c != area_tal_c):

                    if area_tempGrass_c + area_llf_c > 0.75 * area_tal_c:

                        if area_tempGrass_c + area_llf_c > 0.75 * (area_tal_c + area_nc):
                            cat[agri] = 'Category3'

                        elif area_tal_c + area_nc >= 100000 and area_tal_c + area_nc < 300000:
                            cat[agri] = 'Category1_or_3'

                        elif area_tal_c >= 300000:
                            cat[agri] = 'Category2_or_3'

                        elif area_tal_c + area_nc >= 300000:
                            cat[agri] = 'Category1_2_or_3'

                    elif area_tempGrass_c + area_llf_c + area_nc <= 0.75 * (area_tal_c + area_nc):

                        if area_tal_c + area_nc >= 100000 and area_tal_c + area_nc < 300000:
                            cat[agri] = 'Category1'

                        elif area_tal_c >= 300000:
                            cat[agri] = 'Category2'

                        elif area_tal_c + area_nc >= 300000:
                            cat[agri] = 'Category1_or_2'

                    elif area_tal_c + area_nc >= 100000 and area_tal_c + area_nc < 300000:
                        cat[agri] = 'Category1_or_3'

                    elif area_tal_c >= 300000:
                        cat[agri] = 'Category2_or_3'

                    elif area_tal_c + area_nc >= 300000:
                        cat[agri] = 'Category1_2_or_3'

    # --------------------------------------------------------------------------------
    # Check crop diversification rules "CD_diagn"

            if cat[agri] == 'Exemption1' or cat[agri] == 'Exemption2' or cat[agri] == 'Exemption3' or cat[agri] == 'Exemption4':
                cropdiv[agri] = 'Not_required'

            elif cat[agri] == 'Category1':
                if nb_types_c >= 2 and area_mainCrop_c + area_nc <= 0.75*(area_tal_c + area_nc):
                    cropdiv[agri] = 'Compliant'
                elif nb_types_c + nb_parcels_nc < 2 or area_mainCrop_c > 0.75*(area_tal_c + area_nc):
                    cropdiv[agri] = 'Not_compliant'
                else:
                    cropdiv[agri] = 'Missing_info'

            elif cat[agri] == 'Category2':
                if nb_types_c >= 3 and area_mainCrop_c + area_nc <= 0.75*(area_tal_c + area_nc) and area_mainCrop_c + area_2mainCrop_c + area_nc <= 0.95 * (area_tal_c + area_nc):
                    cropdiv[agri] = 'Compliant'
                elif nb_types_c + nb_parcels_nc < 3 or area_mainCrop_c > 0.75*(area_tal_c + area_nc) or area_mainCrop_c + area_2mainCrop_c > 0.95 * (area_tal_c + area_nc):
                    cropdiv[agri] = 'Not_compliant'
                else:
                    cropdiv[agri] = 'Missing_info'

            elif cat[agri] == 'Category3':
                if area_mainCrop_c + area_nc <= 0.75*(area_remAl_ex2_c + area_nc):
                    cropdiv[agri] = 'Compliant'
                elif area_mainCrop_c > 0.75*(area_remAl_ex2_c + area_nc):
                    cropdiv[agri] = 'Not_compliant'
                else:
                    cropdiv[agri] = 'Missing_info'

            elif cat[agri] == 'Category1_or_2':
                if (nb_types_c >= 2 and area_mainCrop_c + area_nc <= 0.75*(area_tal_c + area_nc)) and (nb_types_c >= 3 and area_mainCrop_c + area_nc <= 0.75*(area_tal_c + area_nc) and area_mainCrop_c + area_2mainCrop_c + area_nc <= 0.95 * (area_tal_c + area_nc)):
                    cropdiv[agri] = 'Compliant'
                elif (nb_types_c + nb_parcels_nc < 2 or area_mainCrop_c > 0.75*(area_tal_c + area_nc)) and (nb_types_c + nb_parcels_nc < 3 or area_mainCrop_c > 0.75*(area_tal_c + area_nc) or area_mainCrop_c + area_2mainCrop_c > 0.95 * (area_tal_c + area_nc)):
                    cropdiv[agri] = 'Not_compliant'
                else:
                    cropdiv[agri] = 'Missing_info'

            elif cat[agri] == 'Category1_or_3':
                if (nb_types_c >= 2 and area_mainCrop_c + area_nc <= 0.75*(area_tal_c + area_nc)) and (area_mainCrop_c + area_nc <= 0.75*(area_remAl_ex2_c + area_nc)):
                    cropdiv[agri] = 'Compliant'
                elif (nb_types_c + nb_parcels_nc < 2 or area_mainCrop_c > 0.75*(area_tal_c + area_nc)) and (area_mainCrop_c > 0.75*(area_remAl_ex2_c + area_nc)):
                    cropdiv[agri] = 'Not_compliant'
                else:
                    cropdiv[agri] = 'Missing_info'

            elif cat[agri] == 'Category2_or_3':
                if (nb_types_c >= 3 and area_mainCrop_c + area_nc <= 0.75*(area_tal_c + area_nc) and area_mainCrop_c + area_2mainCrop_c + area_nc <= 0.95 * (area_tal_c + area_nc)) and (area_mainCrop_c + area_nc <= 0.75*(area_remAl_ex2_c + area_nc)):
                    cropdiv[agri] = 'Compliant'
                elif (nb_types_c + nb_parcels_nc < 3 or area_mainCrop_c > 0.75*(area_tal_c + area_nc) or area_mainCrop_c + area_2mainCrop_c > 0.95 * (area_tal_c + area_nc)) and (area_mainCrop_c > 0.75*(area_remAl_ex2_c + area_nc)):
                    cropdiv[agri] = 'Not_compliant'
                else:
                    cropdiv[agri] = 'Missing_info'

            elif cat[agri] == 'Category1_2_or_3':
                if (nb_types_c >= 2 and area_mainCrop_c + area_nc <= 0.75*(area_tal_c + area_nc)) and (nb_types_c >= 3 and area_mainCrop_c + area_nc <= 0.75*(area_tal_c + area_nc) and area_mainCrop_c + area_2mainCrop_c + area_nc <= 0.95 * (area_tal_c + area_nc)) and (area_mainCrop_c + area_nc <= 0.75*(area_remAl_ex2_c + area_nc)):
                    cropdiv[agri] = 'Compliant'
                elif (nb_types_c + nb_parcels_nc < 2 or area_mainCrop_c > 0.75*(area_tal_c + area_nc)) and (nb_types_c + nb_parcels_nc < 3 or area_mainCrop_c > 0.75*(area_tal_c + area_nc) or area_mainCrop_c + area_2mainCrop_c > 0.95 * (area_tal_c + area_nc)) and (area_mainCrop_c > 0.75*(area_remAl_ex2_c + area_nc)):
                    cropdiv[agri] = 'Not_compliant'
                else:
                    cropdiv[agri] = 'Missing_info'

            elif cat[agri] == 'Exemption_or_Category1_or_2':
                if (nb_types_c >= 2 and area_mainCrop_c + area_nc <= 0.75*(area_tal_c + area_nc)) and (nb_types_c >= 3 and area_mainCrop_c + area_nc <= 0.75*(area_tal_c + area_nc) and area_mainCrop_c + area_2mainCrop_c + area_nc <= 0.95 * (area_tal_c + area_nc)):
                    cropdiv[agri] = 'Compliant'
                else:
                    cropdiv[agri] = 'Missing_info'

            elif cat[agri] == 'Exemption_or_Category1_or_3':
                if (nb_types_c >= 2 and area_mainCrop_c + area_nc <= 0.75*(area_tal_c + area_nc)) and (area_mainCrop_c + area_nc <= 0.75*(area_remAl_ex2_c + area_nc)):
                    cropdiv[agri] = 'Compliant'
                else:
                    cropdiv[agri] = 'Missing_info'

            elif cat[agri] == 'Exemption_or_Category2_or_3':
                if (nb_types_c >= 3 and area_mainCrop_c + area_nc <= 0.75*(area_tal_c + area_nc) and area_mainCrop_c + area_2mainCrop_c + area_nc <= 0.95 * (area_tal_c + area_nc)) and (area_mainCrop_c + area_nc <= 0.75*(area_remAl_ex2_c + area_nc)):
                    cropdiv[agri] = 'Compliant'
                else:
                    cropdiv[agri] = 'Missing_info'

            elif cat[agri] == 'Exemption_or_Category1_2_or_3':
                if (nb_types_c >= 2 and area_mainCrop_c + area_nc <= 0.75*(area_tal_c + area_nc)) and (nb_types_c >= 3 and area_mainCrop_c + area_nc <= 0.75*(area_tal_c + area_nc) and area_mainCrop_c + area_2mainCrop_c + area_nc <= 0.95 * (area_tal_c + area_nc)) and (area_mainCrop_c + area_nc <= 0.75*(area_remAl_ex2_c + area_nc)):
                    cropdiv[agri] = 'Compliant'
                else:
                    cropdiv[agri] = 'Missing_info'

            elif cat[agri] == 'Undefined':
                cropdiv[agri] = 'Missing_info'

            else:
                cropdiv[agri] = 'Missing_info'

    # --------------------------------------------------------------------------------
    # Debug factors

            debug[agri]['nb_types_c'] = nb_types_c
            debug[agri]['area_eaa_c'] = area_eaa_c
            debug[agri]['area_tal_c'] = area_tal_c
            debug[agri]['area_tempGrass_c'] = area_tempGrass_c
            debug[agri]['area_permGrass_c'] = area_permGrass_c
            debug[agri]['area_llf_c'] = area_llf_c
            debug[agri]['area_remAl_ex2_c'] = area_remAl_ex2_c
            debug[agri]['area_remAl_ex3_c'] = area_remAl_ex3_c
            debug[agri]['area_mainCrop_c'] = area_mainCrop_c
            debug[agri]['area_2mainCrop_c'] = area_2mainCrop_c
            debug[agri]['nb_parcels_nc'] = nb_parcels_nc
            debug[agri]['area_nc'] = area_nc
            debug[agri]['area_cwater_c'] = area_cwater_c

    # --------------------------------------------------------------------------------
    # Write outputs

        with open(output_csv, 'w') as f_out, open(output_csv.replace('.csv', '_holding.csv'), 'w') as f_out_holding:
            csv_out = csv.DictWriter(f_out, fieldnames=output_fields)
            csv_out.writeheader()
            csv_out_holding = csv.DictWriter(f_out_holding, fieldnames=agri_fields + db_fields)
            csv_out_holding.writeheader()

            for agri in aggDict:
                for fid in aggDict[agri]:
                    row = aggDict[agri][fid]
                    newrow = {}
                    for field in output_fields:
                        if field in output_fields_int:
                            if row[field] == '':
                                newrow[field] = 0
                            else:
                                newrow[field] = int(float(row[field]))
                        elif field in output_fields_float:
                            if row[field] == '':
                                newrow[field] = 0
                            else:
                                newrow[field] = float(row[field])
                        elif field in output_fields_str:
                            newrow[field] = str(row[field])
                    newrow['Classif_r'] = row['Classif_r']
                    newrow['CD_cat'] = cat[agri]
                    newrow['CD_diagn'] = cropdiv[agri]
                    csv_out.writerow(newrow)

                    for field in db_fields:
                        newrow[field] = debug[agri][field]

            for agri in aggDict:
                firstKey = aggDict[agri].keys()[0]
                row = aggDict[agri][firstKey]
                newrow = {}
                for field in agri_fields:
                    if field in output_fields_int:
                        if row[field] == '':
                            newrow[field] = 0
                        else:
                            newrow[field] = int(float(row[field]))
                    elif field in output_fields_float:
                        if row[field] == '':
                            newrow[field] = 0.0
                        else:
                            newrow[field] = float(row[field])
                    elif field in output_fields_str:
                        newrow[field] = str(row[field])
                newrow['CD_cat'] = cat[agri]
                newrow['CD_diagn'] = cropdiv[agri]
                for field in db_fields:
                    newrow[field] = debug[agri][field]
                csv_out_holding.writerow(newrow)

    print('done')


if __name__ == "__main__":
    main()
