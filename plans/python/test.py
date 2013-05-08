# python test imposition plan
# Identity imposition plan


sdoc = params["INPUT"]
tdoc = params["OUTPUT"]
sinfo = pdf_info.extract(sdoc)
bbox = sinfo["page_size"][0][1]
pcount = sinfo["page_count"]
twidth = bbox["width"]
theight = bbox["height"]

imposition_plan = []

for i in range(pcount):
	imposition_plan.append({
		"target_document" : tdoc,
		"target_page_width" : twidth,
		"target_page_height" : theight,
		"pages" : [
			{
			"source_document" : sdoc,
			"source_page" : i,
			"crop_box" : {"left":0,"bottom":0,"width":twidth, "height":theight},
			"translate" : [0,0],
			"rotate" : 0,
			"scale" : [1,1]
			}
			]
	})
