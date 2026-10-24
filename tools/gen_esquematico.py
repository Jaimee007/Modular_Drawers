# -*- coding: utf-8 -*-
import html
COL={'GND':'#333333','5V':'#c0392b','5V_BUS':'#c0392b','3V3':'#d35400','DATA':'#1f6fd1'}
def ncol(n):
    if n.startswith('SENSE'): return '#1e9e57'
    return COL.get(n,'#6c3fb5')
class S:
    def __init__(s,w,h): s.w=w; s.h=h; s.o=[]
    def add(s,x): s.o.append(x)
    def wire(s,pts,c='#222',w=1.6,dash=None):
        d=' '.join(f'{x},{y}' for x,y in pts)
        da=f' stroke-dasharray="{dash}"' if dash else ''
        s.add(f'<polyline points="{d}" fill="none" stroke="{c}" stroke-width="{w}"{da}/>')
    def dot(s,x,y,c='#222'): s.add(f'<circle cx="{x}" cy="{y}" r="3" fill="{c}"/>')
    def t(s,x,y,txt,size=12,anchor='start',c='#222',wt='normal',it=False):
        st=' font-style="italic"' if it else ''
        s.add(f'<text x="{x}" y="{y}" font-size="{size}" text-anchor="{anchor}" fill="{c}" font-weight="{wt}"{st}>{html.escape(txt)}</text>')
    def rect(s,x,y,w,h,fill='#fff',stroke='#222',sw=1.6,rx=0,dash=None):
        da=f' stroke-dasharray="{dash}"' if dash else ''
        s.add(f'<rect x="{x}" y="{y}" width="{w}" height="{h}" fill="{fill}" stroke="{stroke}" stroke-width="{sw}" rx="{rx}"{da}/>')
    def net(s,x,y,name,side='right'):
        c=ncol(name); w=8+7.2*len(name)
        if side=='right':
            pts=f'{x},{y} {x+8},{y-9} {x+w+8},{y-9} {x+w+8},{y+9} {x+8},{y+9}'
            tx=x+10; an='start'
        else:
            pts=f'{x},{y} {x-8},{y-9} {x-w-8},{y-9} {x-w-8},{y+9} {x-8},{y+9}'
            tx=x-10; an='end'
        s.add(f'<polygon points="{pts}" fill="#fff" stroke="{c}" stroke-width="1.4"/>')
        s.t(tx,y+4,name,11,an,c,'bold')
    def res_h(s,x,y,ref,val):
        s.wire([(x,y),(x+8,y)]); s.rect(x+8,y-6,24,12); s.wire([(x+32,y),(x+40,y)])
        s.t(x+20,y-10,ref,10,'middle','#555'); s.t(x+20,y+19,val,10,'middle','#555')
    def res_v(s,x,y,ref,val):
        s.wire([(x,y),(x,y+8)]); s.rect(x-6,y+8,12,24); s.wire([(x,y+32),(x,y+40)])
        s.t(x+10,y+18,ref,10,'start','#555'); s.t(x+10,y+30,val,10,'start','#555')
    def gnd(s,x,y):
        s.wire([(x,y),(x,y+8)]); s.wire([(x-9,y+8),(x+9,y+8)]); s.wire([(x-6,y+12),(x+6,y+12)]); s.wire([(x-3,y+16),(x+3,y+16)])
    def cap_v(s,x,y,ref,val,g=True,pol=False):
        s.wire([(x,y),(x,y+16)]); s.wire([(x-10,y+16),(x+10,y+16)],w=2.4)
        s.wire([(x-10,y+22),(x+10,y+22)],w=2.4); s.wire([(x,y+22),(x,y+38)])
        if pol: s.t(x-16,y+15,'+',11)
        s.t(x+13,y+16,ref,10,'start','#555'); s.t(x+13,y+28,val,10,'start','#555')
        if g: s.gnd(x,y+38)
    def diode_h(s,xa,xc,y,ref,val,below=True):
        # triangle from anode to cathode
        s.wire([(xa,y),(xc,y)])
        d=1 if xc>xa else -1
        m=(xa+xc)/2; a=m-7*d; c=m+7*d
        s.add(f'<polygon points="{a},{y-7} {a},{y+7} {c},{y}" fill="#fff" stroke="#222" stroke-width="1.6"/>')
        s.wire([(c,y-7),(c,y+7)],w=2)
        s.t(m,y-11,ref,10,'middle','#555'); s.t(m,y+20 if below else y-22,val,10,'middle','#555')
    def jumper(s,x,y,ref,note=''):
        s.wire([(x,y),(x+10,y)])
        s.add(f'<path d="M{x+18},{y-6} A6,6 0 0,0 {x+18},{y+6} Z" fill="#c9a227" stroke="#222" stroke-width="1"/>')
        s.add(f'<path d="M{x+22},{y-6} A6,6 0 0,1 {x+22},{y+6} Z" fill="#c9a227" stroke="#222" stroke-width="1"/>')
        s.wire([(x+30,y),(x+40,y)])
        s.t(x+20,y-10,ref,10,'middle','#555')
        if note: s.t(x+20,y+18,note,9,'middle','#888')
    def tp(s,x,y,label):
        s.add(f'<circle cx="{x}" cy="{y}" r="6" fill="#f5d76e" stroke="#222" stroke-width="1.4"/>')
        s.t(x+12,y+4,label,11)
    def switch(s,x,y,ref,val):
        s.wire([(x,y),(x+15,y)]); s.dot(x+15,y); s.dot(x+45,y)
        s.wire([(x+15,y),(x+43,y-12)],w=1.8); s.wire([(x+45,y),(x+60,y)])
        s.t(x+30,y-18,ref,10,'middle','#555'); s.t(x+30,y+18,val,10,'middle','#555')
    def ic(s,x,y,w,h,title,sub,left,right,pitch=40,y0=40,plen=40):
        s.rect(x,y,w,h,fill='#fffbe8',sw=2)
        s.t(x+w/2,y-24,title,13,'middle','#222','bold'); s.t(x+w/2,y-9,sub,10,'middle','#666')
        pl={}
        for i,(num,name) in enumerate(left):
            yy=y+y0+i*pitch; s.wire([(x-plen,yy),(x,yy)])
            s.t(x+6,yy+4,name,11); s.t(x-plen+4,yy-4,str(num),9,'start','#888'); pl[name]=(x-plen,yy)
        for i,(num,name) in enumerate(right):
            yy=y+y0+i*pitch; s.wire([(x+w,yy),(x+w+plen,yy)])
            s.t(x+w-6,yy+4,name,11,'end'); s.t(x+w+plen-4,yy-4,str(num),9,'end','#888'); pl[name]=(x+w+plen,yy)
        return pl
    def conn(s,x,y,title,sub,nets,kind):
        h=len(nets)*24+12
        fill='#eef6ff' if kind=='coleta' else '#f3f3f3'
        s.rect(x,y,70,h,fill=fill,sw=1.8,rx=4)
        if kind=='header':  # muesca de la carcasa
            s.rect(x-4,y+h/2-14,6,28,fill='#fff',sw=1.4)
        s.t(x,y-20,title,12,'start','#222','bold'); s.t(x,y-6,sub,10,'start','#666')
        for i,(pin,n) in enumerate(nets):
            yy=y+18+i*24
            sym='▮' if kind=='header' else '○'
            s.t(x+10,yy+4,f'{pin} {sym}',11)
            s.wire([(x+70,yy),(x+110,yy)]); s.net(x+110,yy,n,'right')
    def svg(s):
        return f'<svg viewBox="0 0 {s.w} {s.h}" xmlns="http://www.w3.org/2000/svg" font-family="Segoe UI, Helvetica, Arial, sans-serif" style="width:100%;height:auto;background:#fff">'+''.join(s.o)+'</svg>'

# ================= HOJA 2: MÓDULO CAJÓN =================
def hoja_cajon():
    s=S(1500,1130)
    # frame
    s.rect(10,10,1480,1110,fill='none',stroke='#999',sw=1)
    s.t(24,36,'HOJA 2 · Módulo cajón 1×1 (idéntico en el cajón #1 y el #2; solo cambian los jumpers de dirección)',16,'start','#222','bold')
    # power
    s.t(40,72,'Alimentación',12,'start','#444','bold')
    s.net(60,100,'5V','left'); s.wire([(60,100),(220,100)]); s.dot(140,100)
    s.cap_v(140,100,'C1','10µF')
    s.rect(220,80,100,50,fill='#fffbe8',sw=2); s.t(270,74,'U2 XC6206P332 · C5446',11,'middle','#222','bold')
    s.t(226,104,'VIN',10); s.t(314,104,'VOUT',10,'end'); s.t(270,126,'GND',10,'middle')
    s.wire([(270,130),(270,140)]); s.gnd(270,140)
    s.wire([(320,100),(500,100)]); s.dot(380,100); s.dot(440,100)
    s.cap_v(380,100,'C2','1µF'); s.cap_v(440,100,'C3','100nF')
    s.net(500,100,'3V3','right')
    s.t(440,175,'C3 junto al pin 9 (VDD)',9,'middle','#888',it=True)
    # connectors
    cn=[('J1','IZQUIERDA · conector macho 2×3 (box header)','SENSE_L','header'),
        ('J2','DERECHA · coleta (6 pads + cable plano + IDC)','SENSE_R','coleta'),
        ('J3','SUPERIOR · coleta (6 pads + cable plano + IDC)','SENSE_T','coleta'),
        ('J4','INFERIOR · conector macho 2×3 (box header)','SENSE_B','header')]
    for i,(r,sub,sn,k) in enumerate(cn):
        y=228+i*190
        s.conn(40,y,r,sub,[('1','GND'),('2','5V'),('3','DATA'),('4',sn),('5','GND'),('6','5V')],k)
    s.t(40,980,'Solo en el 2×1: J5 (SUPERIOR u1, coleta) → SENSE_T2 y',10,'start','#888',it=True)
    s.t(40,996,'J6 (INFERIOR u1, conector macho) → SENSE_B2, mismo pinout.',10,'start','#888',it=True)
    # MCU
    left=[(9,'VDD'),(7,'VSS'),(2,'PD5'),(5,'PA1'),(6,'PA2'),(19,'PD2'),(20,'PD3'),(1,'PD4'),(17,'PC7')]
    right=[(16,'PC6'),(3,'PD6'),(10,'PC0'),(11,'PC1'),(12,'PC2'),(13,'PC3'),(14,'PC4'),(8,'PD0'),(15,'PC5'),(18,'PD1/SWIO'),(4,'PD7/NRST')]
    pl=s.ic(640,230,190,470,'U1  CH32V003F4P6','TSSOP-20 · JLCPCB C5187096',left,right)
    x,y=pl['VDD']; s.wire([(x,y),(x-40,y)]); s.net(x-40,y,'3V3','left')
    x,y=pl['VSS']; s.wire([(x,y),(x-40,y)]); s.net(x-40,y,'GND','left')
    rows=[('PD5','R1','DATA','bus DATA (UART half-duplex)'),('PA1','R2','SENSE_L',''),('PA2','R3','SENSE_R',''),
          ('PD2','R4','SENSE_T',''),('PD3','R5','SENSE_B',''),('PD4','R6','SENSE_T2','solo 2×1'),('PC7','R7','SENSE_B2','solo 2×1')]
    for pin,r,n,note in rows:
        x,y=pl[pin]; s.wire([(x,y),(x-10,y)]); s.res_h(x-50,y,r,'1k'); s.wire([(x-50,y),(x-70,y)]); s.net(x-70,y,n,'left')
        if note: s.t(x-74-8-7.2*len(n)-12,y+4,note,9,'end','#888',it=True)
    # LED
    x,y=pl['PC6']  # 870,270
    s.wire([(x,y),(x+20,y)]); s.res_h(x+20,y,'R8','330Ω'); s.wire([(x+60,y),(1010,y)])
    s.rect(1010,240,120,80,fill='#fffbe8',sw=2)
    s.t(1070,236-0,'',1)
    s.t(1016,274,'DIN',10); s.t(1124,274,'DOUT',10,'end'); s.t(1070,252,'VDD',10,'middle'); s.t(1070,314,'GND',10,'middle')
    s.t(1070,300,'SK6812RGBW',10,'middle','#222','bold')
    s.wire([(1070,320),(1070,330)]); s.gnd(1070,330)
    s.net(900,170,'5V','left'); s.wire([(900,170),(920,170)]); s.diode_h(920,980,170,'D1','1N4148W',below=False)
    s.wire([(980,170),(1150,170)]); s.dot(1070,170); s.wire([(1070,170),(1070,240)])
    s.dot(1150,170); s.cap_v(1150,170,'C4','100nF')
    s.t(1230,110,'LED1 · SKC6812RGBW-BW (C5181320)',11,'start','#222','bold')
    s.t(1230,126,'VDD ≈ 4,3 V gracias a D1, así acepta',10,'start','#666')
    s.t(1230,140,'la señal de 3,3 V del micro.',10,'start','#666')
    s.wire([(1130,280),(1220,280)]); s.t(1226,284,'→ LED2 DIN (solo 2×1)',10,'start','#888',it=True)
    # switch
    x,y=pl['PD6']; s.wire([(x,y),(x+30,y)]); s.net(x+30,y,'SW_CAJON','right')
    s.t(1180,375,'Final de carrera',12,'start','#444','bold')
    s.net(1210,410,'SW_CAJON','left'); s.wire([(1210,410),(1240,410)]); s.switch(1240,410,'SW1','DM1-12UP-1 · C405949'); s.wire([(1300,410),(1360,410)]); s.gnd(1360,410)
    s.t(1180,450,'COM → GND · NO → PD6 (pull-up interna)',10,'start','#666')
    s.t(1180,465,'Pulsado = cajón cerrado = 0 · Abierto = 1',10,'start','#666')
    # jumpers
    names=[('PC0','JP0','×1'),('PC1','JP1','×2'),('PC2','JP2','×4'),('PC3','JP3','×8'),('PC4','JP4','×16'),('PD0','JP5','×32'),('PC5','JP6','tamaño')]
    for pin,j,nt in names:
        x,y=pl[pin]; s.wire([(x,y),(x+10,y)]); s.jumper(x+10,y,j); s.wire([(x+50,y),(x+60,y)])
        s.t(x+64,y+4,'GND',10,'start',COL['GND'],'bold'); s.t(x+96,y+4,nt,10,'start','#888')
    x,y=pl['PD1/SWIO']; s.wire([(x,y),(x+30,y)]); s.tp(x+36,y,'TP1 SWIO (programación)')
    x,y=pl['PD7/NRST']; s.wire([(x,y),(x+30,y)]); s.tp(x+36,y,'TP2 NRST (opcional)')
    s.tp(906,720,'TP3 → 3V3'); s.tp(1036,720,'TP4 → GND')
    s.t(870,745,'TP1+TP3+TP4 = pads para la punta del WCH-LinkE',10,'start','#666')
    # notes box
    s.rect(1110,495,370,205,fill='#f8f9fb',stroke='#ccd',sw=1,rx=6)
    nts=['Notas',
         '• JP0–JP6: puente de soldadura a GND.',
         '   Cerrado = 1; dirección = suma de los pesos.',
         '• JP6 abierto = 1×1 · cerrado = 2×1.',
         '• SENSE: entrada con pull-up interna;',
         '   para avisar, el micro solo lo tira a 0.',
         '• PD5: USART half-duplex en open-drain',
         '   (el pull-up del bus está en la base).',
         '• 5V, GND y DATA de J1–J4, en paralelo.',
         '• Sin condensadores grandes en 5V',
         '   (picos al encajar un cajón en marcha).']
    for i,l in enumerate(nts): s.t(1124,517+i*17.5,l,11 if i else 12,'start','#333','bold' if i==0 else 'normal')
    # config table
    s.t(640,800,'Configuración de los dos cajones del ejemplo',13,'start','#222','bold')
    hdr=['Módulo','JP0 (×1)','JP1 (×2)','JP2–JP5','JP6 (tamaño)','Dirección','Posición (la detecta SENSE)']
    rowsd=[['Cajón #1','cerrado','abierto','abiertos','abierto (1×1)','1','(1,0) · a la derecha de la base'],
           ['Cajón #2','abierto','cerrado','abiertos','abierto (1×1)','2','(2,0) · a la derecha del cajón #1']]
    xs=[640,730,810,890,970,1080,1160]
    s.rect(632,810,850,26,fill='#eef2f7',stroke='#ccd',sw=1)
    for i,hh in enumerate(hdr): s.t(xs[i],828,hh,11,'start','#222','bold')
    for r,row in enumerate(rowsd):
        yy=858+r*26
        for i,v in enumerate(row): s.t(xs[i],yy,v,11,'start','#c0392b' if v=='cerrado' else '#333','bold' if i==0 else 'normal')
    s.wire([(632,842),(1482,842)],c='#ccd',w=1); s.wire([(632,868),(1482,868)],c='#ccd',w=1); s.wire([(632,894),(1482,894)],c='#ccd',w=1)
    # legend
    lg=[('GND',COL['GND']),('5V',COL['5V']),('3V3',COL['3V3']),('DATA',COL['DATA']),('SENSE_x','#1e9e57'),('otras señales','#6c3fb5')]
    s.t(40,1022,'Leyenda de etiquetas de red (mismo nombre = mismo cable):',11,'start','#444','bold')
    for i,(n,c) in enumerate(lg):
        s.rect(40+i*150,1032,14,14,fill=c,stroke=c); s.t(60+i*150,1044,n,11)
    s.t(40,1070,'▮ = pin del conector macho 2×3 (la pestaña blanca marca la muesca)   ○ = pad donde se suelda un hilo de la coleta (hilo rojo = pin 1 = GND)   1k = 1 kΩ',11,'start','#666')
    s.t(40,1094,'Pinout CH32V003F4P6 (TSSOP-20): 1 PD4 · 2 PD5 · 3 PD6 · 4 PD7 · 5 PA1 · 6 PA2 · 7 VSS · 8 PD0 · 9 VDD · 10 PC0 · 11 PC1 · 12 PC2 · 13 PC3 · 14 PC4 · 15 PC5 · 16 PC6 · 17 PC7 · 18 PD1 · 19 PD2 · 20 PD3',10,'start','#888')
    return s.svg()

# ================= HOJA 3: BASE =================
def hoja_base():
    s=S(1500,820)
    s.rect(10,10,1480,800,fill='none',stroke='#999',sw=1)
    s.t(24,36,'HOJA 3 · Módulo base (ESP32-C3 SuperMini + alimentación del bus)',16,'start','#222','bold')
    # USB-C power in
    s.rect(40,140,90,70,fill='#eef6ff',sw=1.8,rx=4); s.t(40,120,'J10 USB-C',12,'start','#222','bold'); s.t(40,134,'breakout solo alimentación',10,'start','#666')
    s.t(48,166,'VBUS',11); s.t(48,196,'GND',11)
    s.wire([(130,162),(170,162)]); s.res_h(170,162,'F1','polyfuse 2 A'); s.wire([(210,162),(420,162)])
    s.wire([(130,192),(150,192)]); s.gnd(150,192)
    s.dot(260,162); s.cap_v(260,162,'C10','1000µF 10V',pol=True)
    # TVS
    s.dot(340,162); s.wire([(340,162),(340,180)])
    s.add('<polygon points="333,180 347,180 340,194" fill="#fff" stroke="#222" stroke-width="1.6"/>')
    s.wire([(333,180),(347,180)],w=2); s.wire([(340,194),(340,200)]); s.gnd(340,200)
    s.t(352,186,'D10 TVS',10,'start','#555'); s.t(352,198,'SMBJ5.0A',10,'start','#555')
    s.dot(420,162); s.net(420,162,'5V','right')
    s.t(40,250,'5V del bus = 5V que reciben todos los cajones',10,'start','#666',it=True)
    # diode to ESP 5V
    s.net(300,330,'5V','left'); s.wire([(300,330),(330,330)]); s.diode_h(330,390,330,'D11','B5819W · C8598')
    s.wire([(390,330),(560,330)])
    s.t(300,380,'D11 evita que el USB de la SuperMini',10,'start','#666',it=True)
    s.t(300,394,'alimente el bus al programarla',10,'start','#666',it=True)
    # ESP
    left=[('','5V'),('','GND'),('','3V3')]
    right=[('','GPIO5 (TX1)'),('','GPIO6 (RX1)'),('','GPIO0'),('','GPIO1'),('','GPIO3'),('','GPIO4'),('','GPIO7'),('','GPIO10')]
    pl=s.ic(600,290,180,360,'U10  ESP32-C3 SuperMini','placa de desarrollo (USB-C propio para programar)',left,right)
    s.wire([(560,330),(560,330)])
    x,y=pl['GND']; s.wire([(x,y),(x-20,y)]); s.net(x-20,y,'GND','left')
    x,y=pl['3V3']; s.wire([(x,y),(x-20,y)]); s.net(x-20,y,'3V3','left')
    # TX via diode
    x,y=pl['GPIO5 (TX1)']; s.wire([(x,y),(x+10,y)]); s.diode_h(x+70,x+10,y,'D12','B5819W'); s.wire([(x+70,y),(x+130,y)]); s.dot(x+130,y)
    x2,y2=pl['GPIO6 (RX1)']; s.wire([(x2,y2),(x+130,y2),(x+130,y)])
    s.wire([(x+130,y),(x+200,y)]); s.net(x+200,y,'DATA','right')
    s.wire([(x+130,y),(x+130,y-60)]); s.res_v(x+130,y-100,'R10','4,7k'); s.wire([(x+130,y-100),(x+130,y-110)]); s.net(x+130,y-110,'3V3','right')
    s.t(x+170,y-30,'TX solo puede tirar DATA a 0 (cátodo hacia TX).',10,'start','#666',it=True)
    s.t(x+170,y-16,'R10 es el único pull-up de todo el bus.',10,'start','#666',it=True)
    for pin,r,n in [('GPIO0','R11','SENSE_L'),('GPIO1','R12','SENSE_R'),('GPIO3','R13','SENSE_T'),('GPIO4','R14','SENSE_B')]:
        x,y=pl[pin]; s.wire([(x,y),(x+10,y)]); s.res_h(x+10,y,r,'1k'); s.wire([(x+50,y),(x+70,y)]); s.net(x+70,y,n,'right')
    x,y=pl['GPIO7']; s.wire([(x,y),(x+20,y)]); s.switch(x+20,y,'','SW10 · final de carrera'); s.wire([(x+80,y),(x+140,y)]); s.gnd(x+140,y)
    x,y=pl['GPIO10']; s.wire([(x,y),(x+10,y)]); s.res_h(x+10,y,'R15','330Ω'); s.wire([(x+50,y),(x+70,y)]); s.t(x+76,y+4,'→ DIN LED10 (SK6812RGBW, mismo circuito que el cajón: D1 + C4)',10,'start','#666')
    s.t(600,690,'Evitar GPIO2, GPIO8 y GPIO9 (pines de arranque; GPIO8 tiene el LED de la placa).',10,'start','#888',it=True)
    # connectors on right
    cn=[('J11','DERECHA · coleta','SENSE_R','coleta'),('J12','SUPERIOR · coleta','SENSE_T','coleta'),
        ('J13','IZQUIERDA · conector macho 2×3','SENSE_L','header'),('J14','INFERIOR · conector macho 2×3','SENSE_B','header')]
    for i,(r,sub,sn,k) in enumerate(cn):
        xx=1100 if i%2==0 else 1290; yy=110 if i<2 else 380
        s.conn(xx,yy,r,sub,[('1','GND'),('2','5V'),('3','DATA'),('4',sn),('5','GND'),('6','5V')],k)
    s.t(40,760,'Igual que en los cajones: GND, 5V y DATA de J11–J14 en paralelo; cada SENSE va a su propio GPIO a través de 1 kΩ.',11,'start','#444')
    return s.svg()

# ================= HOJA 1: INTERCONEXIÓN =================
def hoja_inter():
    s=S(1500,600)
    s.rect(10,10,1480,580,fill='none',stroke='#999',sw=1)
    s.t(24,36,'HOJA 1 · Interconexión: BASE → CAJÓN #1 → CAJÓN #2 (fila inferior, de izquierda a derecha)',16,'start','#222','bold')
    yG,y5,yD,yS=200,240,280,320
    blocks=[(40,'BASE','ESP32-C3 · posición (0,0)'),(560,'CAJÓN #1','CH32V003 · dirección 1 · (1,0)'),(1080,'CAJÓN #2','CH32V003 · dirección 2 · (2,0)')]
    for bx,tt,sub in blocks:
        s.rect(bx,90,380,400,fill='#fcfcfd',stroke='#888',sw=1.6,rx=10)
        s.t(bx+190,116,tt,15,'middle','#222','bold'); s.t(bx+190,134,sub,11,'middle','#666')
    lines=[(yG,'GND',COL['GND']),(y5,'5V',COL['5V']),(yD,'DATA',COL['DATA'])]
    # connectors: right of each block (macho), left of drawers (hembra)
    def cbox(x,kind,label):
        s.rect(x,170,40,170,fill='#eef6ff' if kind=='coleta' else '#f3f3f3',sw=1.6,rx=3)
        s.t(x+20,162,label,10,'middle','#444','bold')
        for pn,yy in zip(['1·5','2·6','3','4'],[yG,y5,yD,yS]):
            s.t(x+20,yy+4,pn,10,'middle','#222','bold')
    # base internals
    bx=40
    s.rect(bx+30,360,150,90,fill='#fffbe8',sw=2,rx=2); s.t(bx+105,385,'ESP32-C3',12,'middle','#222','bold')
    s.t(bx+105,402,'SuperMini',10,'middle','#666'); s.t(bx+170,425,'GPIO1',10,'end'); s.t(bx+40,425,'RX/TX',10)
    s.rect(bx+20,150,110,40,fill='#eef6ff',sw=1.4,rx=3); s.t(bx+75,168,'USB-C 5V',11,'middle','#222','bold'); s.t(bx+75,182,'F1 · C10 · D10',9,'middle','#666')
    cbox(bx+340,'coleta','J11 coleta')
    for yy,n,c in lines:
        x0={'GND':bx+150,'5V':bx+70,'DATA':bx+200}[n]
        s.wire([(x0,yy),(bx+340,yy)],c=c,w=2.4)
    s.wire([(bx+130,170),(bx+150,170),(bx+150,yG)],c=COL['GND'],w=2.4); s.wire([(bx+70,190),(bx+70,y5)],c=COL['5V'],w=2.4)
    # DATA in base: pullup + rx/tx
    s.wire([(bx+200,yD),(bx+110,yD),(bx+110,360)],c=COL['DATA'],w=2.4)
    s.t(bx+116,yD+16,'R10 4,7k → 3V3 · D12',9,'start',COL['DATA'])
    s.wire([(bx+340,yS),(bx+250,yS),(bx+250,420),(bx+180,420)],c='#1e9e57',w=2.4); s.res_h(bx+270,yS,'R12','1k') if False else None
    s.t(bx+256,yS+18,'1k',10,'start','#1e9e57'); s.t(bx+256,yS+32,'SENSE_R',9,'start','#1e9e57')
    # drawer internals
    def drawer(bx,addr):
        cbox(bx,'header','J1 macho'); cbox(bx+340,'coleta','J2 coleta')
        for yy,n,c in lines: s.wire([(bx+40,yy),(bx+340,yy)],c=c,w=2.4)
        s.rect(bx+120,380,140,80,fill='#fffbe8',sw=2,rx=2)
        s.t(bx+190,404,'CH32V003',12,'middle','#222','bold'); s.t(bx+190,420,f'JP → dir {addr}',10,'middle','#666')
        s.t(bx+126,446,'PA1',10); s.t(bx+254,446,'PA2',10,'end'); s.t(bx+190,392+0,'',1)
        # DATA tap
        s.dot(bx+190,yD,COL['DATA']); s.wire([(bx+190,yD),(bx+190,380)],c=COL['DATA'],w=2.4); s.t(bx+186,yD+40,'R1 → PD5',9,'end',COL['DATA'])
        # 5V tap to LDO
        s.dot(bx+132,y5,COL['5V']); s.wire([(bx+132,y5),(bx+132,380)],c=COL['5V'],w=1.6,dash='4 3'); s.t(bx+126,y5+60,'5V → U2',9,'end',COL['5V'])
        s.dot(bx+230,yG,COL['GND']); s.wire([(bx+230,yG),(bx+230,380)],c=COL['GND'],w=1.6,dash='4 3')
        # sense
        s.wire([(bx+40,yS),(bx+80,yS),(bx+80,442),(bx+120,442)],c='#1e9e57',w=2.4); s.t(bx+86,yS+18,'R2 1k',9,'start','#1e9e57')
        s.wire([(bx+340,yS),(bx+300,yS),(bx+300,442),(bx+260,442)],c='#1e9e57',w=2.4); s.t(bx+294,yS+18,'R3 1k',9,'end','#1e9e57')
        s.t(bx+190,478,'J3 (arriba) y J4 (abajo): igual, SENSE_T → PD2, SENSE_B → PD3',9,'middle','#888',it=True)
    drawer(560,1); drawer(1080,2)
    # mating links
    for xa,xb in [(420,560),(940,1080)]:
        s.rect(xa,184,xb-xa-14,152,fill='#e3e6eb',stroke='#b8bec8',sw=1,rx=4)
        s.rect(xb-14,176,14,168,fill='#555',stroke='#333',sw=1,rx=2)
        for yy,n,c in lines+[(yS,'SENSE','#1e9e57')]:
            s.wire([(xa,yy),(xb,yy)],c=c,w=3)
        s.t((xa+xb)/2-7,360,'coleta: cable plano de 6 hilos',10,'middle','#444','bold')
        s.t((xa+xb)/2-7,374,'IDC hembra ▶ conector macho',9,'middle','#666')
        s.t((xa+xb)/2-7,388,'GND y 5V: 2 hilos cada uno',9,'middle','#666',it=True)
        for yy,n,c in lines+[(yS,'SENSE','#1e9e57')]:
            s.t((xa+xb)/2,yy-5,n,10,'middle',c,'bold')
    # descriptive text
    s.t(40,525,'• GND, 5V y DATA son el mismo cable en los tres módulos (bus en paralelo): la base alimenta y habla con todos.',12,'start','#333')
    s.t(40,546,'• SENSE va de punto a punto: el GPIO1 de la base solo llega al PA1 del cajón #1, y el PA2 del cajón #1 solo llega al PA1 del cajón #2.',12,'start','#333')
    s.t(40,567,'• Descubrimiento: la base tira a 0 GPIO1 → el cajón 1 lo ve en PA1 (izquierda) → está en (1,0). El cajón 1 tira a 0 PA2 → el cajón 2 lo ve en PA1 → está en (2,0).',12,'start','#333')
    return s.svg()

page=f'''<!doctype html><html lang="es"><head><meta charset="utf-8"><title>Esquemático · cajonera modular · base + 2 cajones</title>
<style>:root{{color-scheme:light}} body{{font-family:Segoe UI,Helvetica,Arial,sans-serif;background:#eef0f3;margin:0;padding:24px;color:#222}}
.sheet{{background:#fff;max-width:1500px;margin:0 auto 28px;box-shadow:0 1px 4px rgba(0,0,0,.15)}} h1{{max-width:1500px;margin:0 auto 6px;font-size:22px}}
p.sub{{max-width:1500px;margin:0 auto 18px;color:#555}} table{{border-collapse:collapse;width:100%;font-size:13px}} td,th{{border:1px solid #d5d9e0;padding:6px 8px;text-align:left}} th{{background:#eef2f7}}
.box{{background:#fff;max-width:1452px;margin:0 auto 28px;padding:24px;box-shadow:0 1px 4px rgba(0,0,0,.15)}}</style></head><body>
<h1>Esquemático de referencia · cajonera modular (base + 2 cajones 1×1)</h1>
<p class="sub">Revisión 0.2 · 24/10/2026 · Micro CH32V003F4P6, LED SKC6812RGBW. Conexión entre cajones con cable plano de 6 hilos (GND, 5V, DATA, SENSE, GND, 5V): coleta soldada en las caras derecha y superior, conector macho 2×3 en las caras izquierda e inferior. Las redes con el mismo nombre están conectadas.</p>
<div class="sheet">{hoja_inter()}</div>
<div class="sheet">{hoja_cajon()}</div>
<div class="sheet">{hoja_base()}</div>
<div class="box"><h2 style="margin-top:0">Lista de componentes por módulo cajón</h2>
<table><tr><th>Ref.</th><th>Componente</th><th>Valor / referencia</th><th>JLCPCB</th><th>Notas</th></tr>
<tr><td>U1</td><td>Microcontrolador</td><td>CH32V003F4P6 (TSSOP-20)</td><td>C5187096</td><td>Extendida</td></tr>
<tr><td>U2</td><td>LDO 3,3 V</td><td>XC6206P332MR-G (SOT-23)</td><td>C5446</td><td>Básica</td></tr>
<tr><td>LED1</td><td>LED RGBW direccionable</td><td>SKC6812RGBW-BW (5050)</td><td>C5181320</td><td>En el 2×1, añadir LED2 encadenado (DOUT → DIN)</td></tr>
<tr><td>D1</td><td>Diodo alimentación LED</td><td>1N4148W (SOD-123)</td><td>C81598</td><td>Baja VDD del LED a ~4,3 V</td></tr>
<tr><td>SW1</td><td>Final de carrera</td><td>DM1-12UP-1 (THT)</td><td>C405949</td><td>Soldar a mano</td></tr>
<tr><td>R1–R5</td><td>Resistencias serie bus/SENSE</td><td>1 kΩ 0603</td><td>básica</td><td>R6–R7 solo en 2×1</td></tr>
<tr><td>R8</td><td>Resistencia DIN LED</td><td>330 Ω 0603</td><td>básica</td><td></td></tr>
<tr><td>C1</td><td>Entrada 5V</td><td>10 µF 0805</td><td>básica</td><td></td></tr>
<tr><td>C2, C3, C4</td><td>Desacoplo</td><td>1 µF, 100 nF, 100 nF 0603</td><td>básica</td><td>C3 junto a VDD; C4 junto al LED</td></tr>
<tr><td>JP0–JP6</td><td>Puentes de soldadura</td><td>huella SolderJumper 2 pads</td><td>—</td><td>Dirección (JP0–JP5) y tamaño (JP6)</td></tr>
<tr><td>J1, J4</td><td>Conector macho (izquierda, abajo)</td><td>Box header IDC 2×3, 2,54 mm, recto</td><td>sin verificar</td><td>J6 extra en el 2×1. Muesca hacia el centro de la placa</td></tr>
<tr><td>J2, J3</td><td>Coleta (derecha, arriba)</td><td>6 pads + ~70 mm de cable plano de 6 hilos (1,27 mm) + conector hembra IDC FC-6P</td><td>—</td><td>J5 extra en el 2×1. Hilo rojo al pad 1 (GND)</td></tr>
<tr><td>TP1–TP4</td><td>Pads de programación</td><td>SWIO, NRST, 3V3, GND</td><td>—</td><td>Para el WCH-LinkE</td></tr></table></div>
</body></html>'''
import os
out=os.path.join(os.path.dirname(os.path.abspath(__file__)),'..','docs','esquematico_2_cajones.html')
open(out,'w',encoding='utf-8').write(page)
print('ok')
