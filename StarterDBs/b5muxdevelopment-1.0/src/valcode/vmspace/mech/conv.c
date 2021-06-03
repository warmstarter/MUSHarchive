/* Test code: cart_to_hex
   Input: a pair of cartesian coordinates
   Output: a variable of type hex_coords which holds the hex coords
*/

#include <stdio.h>

typedef struct hex_coords {
  int x, y;
  };

typedef struct cart_coords {
  float x, y;
  };

struct hex_coords cart_to_hex(cart_x, cart_y)
float cart_x, cart_y;
{
  struct hex_coords hex;
  float x, y, alpha=0.288675134, root_3 = 1.732050808;
  int x_count, y_count, x_offset, y_offset;
  
  if (cart_x < alpha) {
    x_count = -2;
    x = cart_x + 5 * alpha;
    }
  else {
    x_count = (int) ((cart_x - alpha) / root_3);
    x = cart_x - alpha - x_count * root_3;
    }

  y_count = (int) cart_y;
  y = cart_y - y_count;

  printf("x,y = %5.3f,%5.3f ; x_count,y_count = %d,%d\n\n",
	 x,y,x_count,y_count);

  if ((x >= 0.0) && (x < (2.0 * alpha))) {
    printf("Region I\n");
    x_offset = 0;
    y_offset = 0;
    }

  else if ((x >= (3.0 * alpha)) && (x < (5.0 * alpha))) {
    if ((y >= 0.0) && (y < 0.5)) {
    printf("Region II\n");
      x_offset = 1;
      y_offset = 0;
      }
    else {
    printf("Region III\n");
      x_offset = 1;
      y_offset = 1;
      }
    }

  else if ((x >= 2.0 * alpha) && (x < (3.0 * alpha))) {
    if ((y >= 0.0) && (y < 0.5)) { 
      if ((2 * alpha * y) <= (x - 2.0 * alpha)) {
    printf("Region IV\n");
        x_offset = 1;
        y_offset = 0;
        }
      else {
    printf("Region V\n");
        x_offset = 0;
        y_offset = 0;
        }
      }
    else if ((2 * alpha * (1.0 - y)) > (x - 2.0 * alpha)) {
    printf("Region VI\n");
      x_offset = 0;
      y_offset = 0;
      }
    else {
    printf("Region VII\n");
      x_offset = 1;
      y_offset = 1;
      }
    }

  else if ((y >= 0.0) && (y < 0.5)) {
      if ((2 * alpha * y) <= (11.0 * alpha - x - 5.0 * alpha)) {
    printf("Region VIII\n");
        x_offset = 1;
        y_offset = 0;
        }
      else {
    printf("Region IX\n");
        x_offset = 2;
        y_offset = 0;
        }
      }
  else if ((2 * alpha * (y - 0.5) ) <= (x - 5.0 * alpha)) {
    printf("Region X - x,y = %5.3f, %5.3f\n", (x - 5.0 * alpha), (y - 0.5));
    x_offset = 2;
    y_offset = 0;
    }
  else {
    printf("Region XI - x,y = %5.3f, %5.3f\n", (x - 5.0 * alpha), (y - 0.5));
    x_offset = 1;
    y_offset = 1;
    }


  hex.x = x_count * 2 + x_offset;
  hex.y = y_count + y_offset;

  return (hex);
  }

/* if hex_x is even, new_y = (hex_y + 1) * 0.5,
                     new_x = hex_x * 3 alpha + 2 alpha.
   if hex_x is odd, then new_x = (hex_x - 1) * 3 alpha + 5 alpha,
                         new_y = hex_y. 
*/

struct cart_coords hex_to_cart(hex_x, hex_y)
int hex_x, hex_y;
{
  struct cart_coords cart;
  float alpha=0.288675134;

  if (hex_x + 2 - (hex_x + 2) / 2 * 2) { /* hex_x is odd */
    cart.x = (float) (hex_x - 1.0) * 3.0 * alpha + 5.0 * alpha;
    cart.y = (float) (hex_y);
    }
  else {
    cart.x = (float) (hex_x) * 3.0 * alpha + 2.0 * alpha;
    cart.y = (float) hex_y  + 0.5;
    }

  return(cart);
  }
   
main()
{
  struct hex_coords hex;
  struct cart_coords cart;
  float x, y;

  printf("Conversion test........\n\n\n");

  for (;;) {
    printf("Enter x, y coords as xxx, yyy: ");
    scanf("%f, %f", &x, &y);

    hex = cart_to_hex(x, y);

    printf("Coordinates (%5.3f, %5.3f) are in hex %d, %d\n",
      x, y, hex.x, hex.y);

    cart = hex_to_cart(hex.x, hex.y);

    printf("The center of hex (%d, %d) is at (%5.3f, %5.3f)\n\n",
      hex.x, hex.y, cart.x, cart.y);
    }
  }

    
