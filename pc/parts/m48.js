avr_sig['1E9205'] = ['m48a', 'm48', 2048, 256, 32, 64, 4, 64, 5, 4];
avr_sig['1E920A'] = ['m48pa', 'm48', 2048, 256, 32, 64, 4, 64, 5, 4];
avr_sig['1E930A'] = ['m88a', 'm88', 4096, 512, 32, 128, 4, 128, 5, 4];
avr_sig['1E930F'] = ['m88pa', 'm88', 4096, 512, 32, 128, 4, 128, 5, 4];
avr_sig['1E9406'] = ['m168a', 'm88', 8192, 512, 64, 128, 4, 128, 5, 4];
avr_sig['1E940B'] = ['m168pa', 'm88', 8192, 512, 64, 128, 4, 128, 5, 4];
//avr_sig['1E9514'] = ['m328', 'm328', 16384, 1024, 64, 256, 4, 256, 5, 4];
avr_sig['1E9514'] = ['m328', 'm328', 2048, 256, 32, 64, 4, 64, 5, 4];
avr_sig['1E950F'] = ['m328p', 'm328', 16384, 1024, 64, 256, 4, 256, 5, 4];

avr_nvme.m48 = {
	"lock": {"LB_PLACEHOLDER": [6, 63], "LB": [2, 3]},
	"efuse": {"EFUSE_PLACEHOLDER": [7, 127], "SELFPRGEN": [1, 1]},
	"hfuse": {"RSTDISBL": [1, 1], "DWEN": [1, 1], "SPIEN": [1, 0], "WDTON": [1, 1], "EESAVE": [1, 1], "BODLEVEL": [3, 7]},
	"lfuse": {"CKDIV8": [1, 0], "CKOUT": [1, 1], "SUT": [2, 2], "CKSEL": [4, 2]},
	'desc': {
		'LB_PLACEHOLDER': {'63':'RESERVED'},
		'LB': {
			'0': 'Flash & EEP programming + verification disabled',
			'1': 'Flash & EEP programming disabled',
			'3': 'Flash & EEP unlocked'
		},
		'EFUSE_PLACEHOLDER': {'127':'RESERVED'},
		'SELFPRGEN': {
			'0': 'Self programming enabled',
			'1': 'Self programming disabled'
		},
		'RSTDISBL': {
			'0': 'Reset pin as GPIO (use HV to reset)',
			'1': 'Reset pin as reset'
		},
		'DWEN': {
			'0': 'Debug wire enabled',
			'1': 'Debug wire disabled'
		},
		'SPIEN': {
			'0': 'Serial programming enabled',
			'1': 'Serial programming disabled'
		},
		'WDTON': {
			'0': 'Watchdog always on',
			'1': 'Watchdog not on'
		},
		'EESAVE': {
			'0': 'Preserve EEPROM during chip earse',
			'1': 'Chip earse will earse EEPROM'
		},
		'BODLEVEL': {
			'4': '4.1V / 4.3V / 4.5V',
			'5': '2.5V / 2.7V / 2.9V',
			'6': '1.7V / 1.8V / 2.0V',
			'7': 'Disabled'
		},
		'CKDIV8': {
			'0': 'CPU speed at CLK/8',
			'1': 'CPU speed at CLK'
		},
		'CKOUT': {
			'0': 'Output clock signal on pin',
			'1': 'Do not output clock'
		},
		'SUT': {
			'0': 'Fast start-up',
			'1': '4ms delay',
			'2': '64ms delay'
		},
		'CKSEL': {
			'0': 'External clock',
			'2': '8MHz internal RC',
			'3': '128kHz internal',
			'4': '32kHz crystal, fast start-up',
			'5': '32kHz crystal, slow start-up',
			'6': 'Full swing crystal (see doc p.40)',
			'7': 'Full swing crystal (see doc p.40)',
			'8': '0.4-0.9 MHz external crystal, fast start-up',
			'9': '0.4-0.9 MHz external crystal, slow start-up',
			'10': '0.9-3.0 MHz external crystal, fast start-up',
			'11': '0.9-3.0 MHz external crystal, slow start-up',
			'12': '3-8 MHz external crystal, fast start-up',
			'13': '3-8 MHz external crystal, slow start-up',
			'14': '8-16 MHz external crystal, fast start-up',
			'15': '8-16 MHz external crystal, slow start-up'
		}
	},
	'doc': 'https://ww1.microchip.com/downloads/aemDocuments/documents/MCU08/ProductDocuments/DataSheets/ATmega48A-PA-88A-PA-168A-PA-328-P-DS-DS40002061B.pdf'
};

avr_nvme.m88 = {
	"lock": {"LB_PLACEHOLDER": [2, 3], "BLB1": [2, 3], "BLB0": [2, 3], "LB": [2, 3]},
	"efuse": {"EFUSE_PLACEHOLDER": [5, 31], "BOOTSZ": [2, 0], "BOOTRST": [1, 1]},
	"hfuse": avr_nvme.m48.hfuse,
	"lfuse": avr_nvme.m48.lfuse,
	'desc': {
		'LB_PLACEHOLDER': {'3':'RESERVED'},
		'BLB1': {
			'0': 'No SPM to Application, No LPM from Bootloader to Application',
			'1': 'No LPM from Bootloader to Application',
			'2': 'No SPM to Application',
			'3': 'Free SPM / LPM access to Application'
		},
		'BLB0': {
			'0': 'No SPM to Bootloader, No LPM from Application to Bootloader',
			'1': 'No LPM from Application to Bootloader',
			'2': 'No SPM to Bootloader',
			'3': 'Free SPM / LPM access to Bootloader'
		},
		'LB': avr_nvme.m48.desc.LB,
		'EFUSE_PLACEHOLDER': {'31':'RESERVED'},
		'BOOTSZ': {
			'0': '2048 words Bootloader',
			'1': '1024 words Bootloader',
			'2': '512 words Bootloader',
			'3': '256 words Bootloader'
		},
		'BOOTRST': {
			'0': 'Reset from Bootloader section',
			'1': 'Reset from Application section'
		},
		'RSTDISBL': avr_nvme.m48.desc.RSTDISBL,
		'DWEN': avr_nvme.m48.desc.DWEN,
		'SPIEN': avr_nvme.m48.desc.SPIEN,
		'WDTON': avr_nvme.m48.desc.WDTON,
		'EESAVE': avr_nvme.m48.desc.EESAVE,
		'BODLEVEL': avr_nvme.m48.desc.BODLEVEL,
		'CKDIV8': avr_nvme.m48.desc.CKDIV8,
		'CKOUT': avr_nvme.m48.desc.CKOUT,
		'SUT': avr_nvme.m48.desc.SUT,
		'CKSEL': avr_nvme.m48.desc.CKSEL
	},
	'doc': avr_nvme.m48.doc
};

avr_nvme.m328 = {
	"lock": avr_nvme.m88.lock,
	"efuse": {"EFUSE_PLACEHOLDER": [5, 31], "BODLEVEL": [3, 7]},
	"hfuse": {"RSTDISBL": [1, 1], "DWEN": [1, 1], "SPIEN": [1, 0], "WDTON": [1, 1], "EESAVE": [1, 1], "BOOTSZ": [2, 0], "BOOTRST": [1, 1]},
	"lfuse": avr_nvme.m48.lfuse,
	"desc": {
		'LB_PLACEHOLDER': avr_nvme.m88.desc.LB_PLACEHOLDER,
		'BLB1': avr_nvme.m88.desc.BLB1,
		'BLB0': avr_nvme.m88.desc.BLB0,
		'LB': avr_nvme.m48.desc.LB,
		'EFUSE_PLACEHOLDER': avr_nvme.m88.desc.EFUSE_PLACEHOLDER,
		'BODLEVEL': avr_nvme.m48.desc.BODLEVEL,
		'RSTDISBL': avr_nvme.m48.desc.RSTDISBL,
		'DWEN': avr_nvme.m48.desc.DWEN,
		'SPIEN': avr_nvme.m48.desc.SPIEN,
		'WDTON': avr_nvme.m48.desc.WDTON,
		'EESAVE': avr_nvme.m48.desc.EESAVE,
		'BOOTSZ': avr_nvme.m88.desc.BOOTSZ,
		'BOOTRST': avr_nvme.m88.desc.BOOTRST,
		'CKDIV8': avr_nvme.m48.desc.CKDIV8,
		'CKOUT': avr_nvme.m48.desc.CKOUT,
		'SUT': avr_nvme.m48.desc.SUT,
		'CKSEL': avr_nvme.m48.desc.CKSEL
	},
	'doc': avr_nvme.m48.doc
};