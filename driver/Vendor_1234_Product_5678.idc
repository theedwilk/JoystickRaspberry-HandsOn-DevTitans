# Configuração para Driver GPIO Xbox Customizado
# Vendor: 0x1234, Product: 0x5678

# Define que é um dispositivo externo (joystick plugado/conectado)
# Mesmo sendo GPIO interno, definir como 0 ajuda na detecção de Player em jogos
device.internal = 0

# Vincula explicitamente ao arquivo .kl criado acima
keyboard.layout = Vendor_1234_Product_5678

# Joysticks não devem ser afetados pela rotação da tela
keyboard.orientationAware = 0

# Informa que não é um teclado embutido (evita sumir com o teclado virtual)
keyboard.builtIn = 0