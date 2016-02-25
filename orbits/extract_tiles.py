import xml.etree.ElementTree as ET
from lxml import etree


def get_wkt(srs, coords):
    return "POLYGON(({} {}, {} {}, {} {}, {} {}, {} {}))".format(
            coords[0], coords[1],
            coords[2], coords[3],
            coords[4], coords[5],
            coords[6], coords[7],
            coords[0], coords[1])

tree = etree.parse('S2A_OPER_GIP_TILPAR_MPC__20140923T000000_V20000101T000000_20200101T000000_B00.xml')
root = tree.getroot()
tile_list = root.find('DATA').find('REPRESENTATION_CODE_LIST').find('TILE_LIST')

tiles = []
for tile in tile_list.findall('TILE'):
    tile_id = tile.find('TILE_IDENTIFIER').text
    srs = tile.find('HORIZONTAL_CS_CODE').text
    bbox = tile.find('B_BOX').text
    ulx = float(tile.find('ULX').text)
    uly = float(tile.find('ULY').text)

    found = False
    for tile_size in tile.find('TILE_SIZE_LIST').findall('TILE_SIZE'):
        if tile_size.get('resolution') == '10':
            found = True
            nrows = float(tile_size.find('NROWS').text)
            ncols = float(tile_size.find('NCOLS').text)
            xdim = float(tile_size.find('XDIM').text)
            ydim = float(tile_size.find('YDIM').text)

    urx = ulx + ncols * xdim
    ury = uly

    llx = ulx
    lly = uly + nrows * ydim

    lrx = ulx + ncols * xdim
    lry = uly + nrows * ydim

    if not found:
        raise Exception('Cannot find 10m tile size', tile_id)

    if srs.startswith("EPSG:"):
        srid = srs[5:]
    else:
        raise Exception('Unknown SRID', tile_id, srs)

    wkt = get_wkt(srs, [ulx, uly, llx, lly, lrx, lry, urx, ury])
    tiles.append((tile_id, wkt, srid))

print("BEGIN TRANSACTION;")
print("CREATE TABLE shape_tiles_s2(\n"
      "    tile_id CHAR(5) NOT NULL PRIMARY KEY,\n"
      "    geom GEOMETRY NOT NULL,\n"
      "    geog GEOGRAPHY NOT NULL);\n")
for tile in tiles:
    sql = ("INSERT INTO shape_tiles_s2(tile_id, geom, geog)\n"
           "VALUES('{}',\n"
           "       ST_GeomFromText('{}', {}),\n"
           "       ST_GeomFromText('POLYGON EMPTY'));").format(
            tile[0], tile[1], tile[2])
    print(sql)
print("UPDATE shape_tiles_s2 SET geog = ST_Transform(geom, 4326);")
print("COMMIT;")

