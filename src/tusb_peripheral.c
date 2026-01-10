#include "common/tusb_types.h"
#include "tusb.h"
#include "tusb_option.h"

int tusb_peripheral_init() {
  tusb_rhport_init_t dev_init = {
      .role = TUSB_ROLE_DEVICE,
      .speed = TUSB_SPEED_AUTO,
  };

  tusb_init(0, &dev_init);
  return 0;
}
