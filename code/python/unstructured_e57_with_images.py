import json

import numpy as np
import pye57

from utils import get_point_cloud, write_image_e57
from PIL import Image

# JSON that contains information about the image to be inserted in the E57 (path, rotation, translation, etc.).
pano_image_json = r"image.json"
with open(pano_image_json, "r") as file:
    data_json = json.load(file)

pcd_path = r"../../samples/unstructured_file_wo_image.las"

points = get_point_cloud(pcd_path)

translation_pcd = np.array([0.0, 0.0, 0.0]) # Translation to be applied on the point cloud of the scan
rotation_pcd = np.array([1.0, 0.0, 0.0, 0.0]) # Rotation quaternion to be applied on the point cloud of the scan

with pye57.E57("unstructured_file.e57", mode="w") as e57_write:
    e57_write.write_scan_raw(points, translation=translation_pcd, rotation=rotation_pcd)
    scan_header = e57_write.get_header(0)
    for data in data_json:
        pano_image = Image.open(data["path"])
        translation_image = np.array([data["pos_x"], data["pos_y"], data["pos_z"]]) # Position of the image
        rotation_image = np.array([data["rot_w"], data["rot_x"], data["rot_y"],data["rot_z"]])  # The rotation to be applied on the image
        write_image_e57(e57_write.root["images2D"], e57_write.image_file, pano_image,
                        translation=translation_image, rotation=rotation_image, image_name=data["name"],
                        pixel_width=data["pixelWidth"], pixel_height=data["pixelHeight"])
