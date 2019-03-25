#!/usr/bin/env python3

def get_firstChild(node):
	child = node.firstChild
	while child.nodeType != 1:
		child = child.nextSibling
	if child.nodeType == 1:
		return child
	else:
		return None
		#exit("ERROR: get_firstChild()")

def get_nameChild(node, name):
	child = node.firstChild
	while 1:
		if child.nodeType == 1:
			if child.nodeName == name:
				return child
		if child == node.lastChild:
			return None
			#exit("ERROR: get_nameChild()")
		child = child.nextSibling

def get_elementValue(parent, name):
	node_list = parent.getElementsByTagName(name)
	if node_list.length == 0:
		exit("ERROR: get_elementValue()")
	else:
		return node_list[0].childNodes[0].nodeValue

def build_modulePatch(module, version):
	file_name = patch_dir + module + "_" + version + ".c"
	if os.access(file_name, os.F_OK):
		return
	f = open(file_name, "w")
	print("reg __attribute__((weak)) %s_%s[] = {};\n" % (module, version), file = f)
	f.close()

def atdf_correctRegisterName(name, module_name):
	i = name.find(module_name + "_")
	if i == 0:
		return name[len(module_name)+1:]
	elif i == 1:
		if module_name == "SMC":
			return name[5:]
		else:
			print(name.find(module_name + "_"), name, module_name)
	elif i == 2:
		if module_name == "PIO":
			name = name[6:]
			return "S_" + name
		else:
			print(name.find(module_name + "_"), name, module_name)
	elif i == -1:
		if module_name == "FLEXCOM":
			return name[5:]
		elif module_name == "SHDWC":
			return name[5:]
		elif module_name == "GPBR":
			return name
		elif module_name == "USART":
			return name[3:]
		elif module_name == "PMC":
			return name
		elif module_name == "ISC":
			return name
		elif module_name == "SYSCWP":
			return name
		else:
			print(name.find(module_name + "_"), name, module_name)
	else:
		print(name.find(module_name + "_"), name, module_name)
	return None

def atdf_registerReadClear(register, access):
	bitfield_list = register.getElementsByTagName("bitfield")
	for bitfield in bitfield_list:
		caption = bitfield.getAttribute("caption")
		if caption == None:
			return access
		if caption.find("(clear on read)") != -1:
			return access + "|C"
		if caption.find("(cleared on read)") != -1:
			return access + "|C"
	return access

def atdf_moduleExist(modules, module_name, module_version):
	module = modules.firstChild
	while 1:
		if module.nodeType == 1:
			name    = module.getAttribute("name")
			version = module.getAttribute("version")
			id      = module.getAttribute("id")
			# Workaround for module version
			if id != None and id != name:
				version = id + version
			if name == module_name and version == module_version:
				return module
		if module == modules.lastChild:
			return None
		module = module.nextSibling

def atdf_variants(variants):
	variant = get_nameChild(variants, "variant")
	print(variant.getAttribute("ordercode"))

def atdf_devices(devices, modules):
	device = get_nameChild(devices, "device")
	name = device.getAttribute("name")
	file_name = chip_dir + name + ".c"
	f = open(file_name, "w")
	print("ip %s_ips[]=\n{" % (name), file = f)
	peripherals = get_nameChild(device, "peripherals")
	module_list = peripherals.getElementsByTagName("module")
	for module in module_list:
		name    = module.getAttribute("name")
		version = module.getAttribute("version")
		id      = module.getAttribute("id")
		# Workaround for module version
		if id != None and id != name:
			version = id + version
		instance_list = module.getElementsByTagName("instance")
		for instance in instance_list:
			register_group = get_nameChild(instance, "register-group")
			name   = register_group.getAttribute("name")
			offset = register_group.getAttribute("offset") 
			module = register_group.getAttribute("name-in-module")
			print("\t{\"%s\", %s, \"%s\", &%s_%s}," % (name, offset, version, module, version), file = f)
			if not atdf_moduleExist(modules, module, version):
				#print("WARRING: module missing %s_%s" % (module, version))
				build_modulePatch(module, version)
	print("\t{NULL}", file = f)
	print("};\n", file = f)
	f.close()

def atdf_modules(modules):
	module_list = modules.getElementsByTagName("module")
	for module in module_list:
		module_name = module.getAttribute("name")
		caption = module.getAttribute("caption")
		version = module.getAttribute("version")
		id      = module.getAttribute("id")
		# Workaround for module version
		if id != None and id != module_name:
			version = id + version
		file_name = module_dir + "/" + module_name + "_" + version + ".c"
		if os.access(file_name, os.F_OK):
			continue
		f = open(file_name, "w")
		print("/* %s */" % (caption), file = f)
		print("reg %s_%s[]=\n{" % (module_name, version), file = f)
		register_list = module.getElementsByTagName("register")
		for register in register_list:
			name   = register.getAttribute("name")
			offset = register.getAttribute("offset")
			access = register.getAttribute("rw")
			name   = atdf_correctRegisterName(name, module_name)
			access = atdf_registerReadClear(register, access)
			print("\t{\"%s\", %s, %s}," % (name, offset, access), file = f)
		print("\t{NULL}", file = f)
		print("};\n", file = f)
		f.close()

def process_atdf(file):
	DOMTree = xml.dom.minidom.parse(file)
	root = DOMTree.documentElement
	i= 0
	while 1:
		node = root.childNodes[i]
		if node.localName == "variants":
			variants = node
		if node.localName == "devices":
			devices = node
		if node.localName == "modules":
			modules = node
		if node == root.lastChild:
			break
		else:
			i = i + 1
#	atdf_variants(variants)
	atdf_devices(devices, modules)
	atdf_modules(modules)
	DOMTree.unlink()

def svd_correctModuleName(name, module_name):
	if module_name == "EBI":
		if name == "MPDDRC":
			return name
		elif name == "SMC":
			return name
		elif name == "HSMC":
			return "SMC"
		elif name == "PMECC":
			return name
		elif name == "PMERRLOC":
			return name
		elif name == "DDRSDRC":
			return name
	elif module_name == "SAIC":
		return "AIC"
	elif module_name == "SYSC":
		return name
	return module_name

def svd_correctAccessType(type):
	if type == "read-only":
		return "R"
	elif type == "write-only":
		return "W"
	elif type == "read-write":
		return "RW"
	else:
		exit("ERROR: unknown access type in svd file")

def svd_registerReadClear(register, access):
	fields = get_nameChild(register, "fields")
	if fields == None:
		return access;
	field_list = fields.getElementsByTagName("field")
	if field_list == None:
		return access;
	for field in field_list:
		node_list = field.getElementsByTagName("description")
		if node_list.length == 0:
			return access;
		if node_list[0].childNodes[0].nodeValue.find("(clear on read)") != -1:
			return access + "|C"
		if node_list[0].childNodes[0].nodeValue.find("(cleared on read)") != -1:
			return access + "|C"
	return access

def svd_registers(module, version, description, registers):
	file_name = module_dir + "/" + module + "_" + version + ".c"
	if os.access(file_name, os.F_OK):
		return
	f = open(file_name, "w")
	print("/* %s */" % (description), file = f)
	print("reg %s_%s[]=\n{" % (module, version), file = f)
	register_list = registers.getElementsByTagName("register")
	for register in register_list:
		name   = get_elementValue(register, "name")
		offset = get_elementValue(register, "addressOffset")
		access = svd_correctAccessType(get_elementValue(register, "access"));
		access = svd_registerReadClear(register, access)
		dim    = get_nameChild(register, "dim")
		if dim != None:
			name = name.split('[')[0]
			dim_count  = int(get_elementValue(register, "dim"), base=0)
			dim_inc    = int(get_elementValue(register, "dimIncrement"), base=0)
			dim_start  = int(get_elementValue(register, "dimIndex").split('-')[0], base=0)
			dim_offset = int(offset, base=0)
			i = 0
			while i < dim_count:
				print("\t{\"%s%d\", 0x%08X, %s}," % (name, i, dim_offset, access), file = f)
				dim_offset = dim_offset + dim_inc
				i = i + 1
		else:
			print("\t{\"%s\", %s, %s}," % (name, offset, access), file = f)
	print("\t{NULL}", file = f)
	print("};\n", file = f)
	f.close()

def process_svd(file):
	DOMTree = xml.dom.minidom.parse(file)
	root = DOMTree.documentElement
	device_name = get_elementValue(root, "name")
	file_name = chip_dir + device_name + ".c"
	f = open(file_name, "w")
	print("ip %s_ips[]=\n{" % (device_name), file = f)
	peripherals = get_nameChild(root, "peripherals")
	i=0
	while 1:
		node = peripherals.childNodes[i]
		if node.localName == "peripheral":
			name        = get_elementValue(node, "name")
			version     = get_elementValue(node, "version")
			description = get_elementValue(node, "description")
			address     = get_elementValue(node, "baseAddress")
			group       = node.getElementsByTagName("groupName")
			if group.length == 0:
				# if groupName is null, use name instead of
				module = name
			else:
				module = group[0].childNodes[0].nodeValue
			module = svd_correctModuleName(name, module)
			print("\t{\"%s\", %s, \"%s\", &%s_%s}," % (name, address, version, module, version), file = f)
			registers = get_nameChild(node, "registers")
			if registers != None:
				svd_registers(module, version, description, registers)
			else:
				#print("WARRING: module missing %s_%s" % (module, version))
				build_modulePatch(module, version)
		if node == peripherals.lastChild:
			break;
		else:
			i = i + 1
	print("\t{NULL}", file = f)
	print("};\n", file = f)
	f.close()
	DOMTree.unlink()

import sys
import os
from xml.dom.minidom import parse
import xml.dom.minidom

# main()
argc = len(sys.argv)
if argc < 3:
	exit("Usage:\n\t%s xml_dir out_dir" % (sys.argv[0]))

xml_dir = sys.argv[1]
out_dir  = sys.argv[2]

if os.listdir(out_dir):
	exit("ERROR: %s is not empty!" % (out_dir))
out_dir = out_dir.rstrip('/') + "/"

if not os.path.exists(xml_dir):
	exit("ERROR: %s does not exist!" % (xml_dir))
xml_dir = xml_dir.rstrip('/') + "/"

chip_dir   = out_dir + "chip" + "/"
os.mkdir(chip_dir)
module_dir = out_dir + "module" + "/"
os.mkdir(module_dir)
patch_dir  = out_dir + "module_patch" + "/"
os.mkdir(patch_dir)

print("Enter directory", xml_dir)
for root, dirs, files in os.walk(xml_dir):
	files.sort()
	# Process atdf first
	for file in files:
		if os.path.splitext(file)[1] == ".atdf":
			print("Processing", file)
			process_atdf(xml_dir + file)
	for file in files:
		if os.path.splitext(file)[1] == ".svd":
			print("Processing", file)
			process_svd(xml_dir + file)
print("Leave directory", xml_dir)

print("Enter directory", out_dir)
print("Merge chip files")
chips = os.listdir(chip_dir)
chips.sort()
f_chips = open(out_dir + "chips.c", "w")
for chip in chips:
	if os.path.isfile(chip_dir + chip):
		f_chip = open(chip_dir + chip)
		f_chips.write(f_chip.read())
		f_chip.close()
		os.remove(chip_dir + chip)
		#print(chip, "merged")
f_chips.close()
os.rmdir(chip_dir)

patches = os.listdir(patch_dir)
if patches:
	print("Merge module patch files")
	print("WARRING: following modules missing, check modules_patch.c for more information!")
	patches.sort()
	f_patch = open(out_dir + "modules_patch.c", "w")
	print("/*", file = f_patch)
	print(" * Following modules' register description could not be found in xml files", file = f_patch)
	print(" * We temporarily define these variables as weak symbol for compilation success", file = f_patch)
	print(" * Need to generate register description for these modules manually", file = f_patch)
	print(" */\n", file = f_patch)
	for patch in patches:
		if os.path.isfile(patch_dir + patch):
			if not os.access(module_dir + patch, os.F_OK):
				print("  Missing", patch.rstrip('.c'))
				f = open(patch_dir + patch)
				f_patch.write(f.read())
				f.close()
				#print(patch, "merged")
			os.remove(patch_dir + patch)
	f_patch.close()
os.rmdir(patch_dir)

print("Merge module files")
modules = os.listdir(module_dir)
modules.sort()
f_modules = open(out_dir + "modules.c", "w")
for module in modules:
	if os.path.isfile(module_dir + module):
		f_module = open(module_dir + module)
		f_modules.write(f_module.read())
		f_module.close()
		os.remove(module_dir + module)
f_modules.close()
os.rmdir(module_dir)
print("Leave directory", out_dir)

exit(0)

