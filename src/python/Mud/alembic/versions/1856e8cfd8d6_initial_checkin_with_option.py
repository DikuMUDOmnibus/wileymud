"""Initial checkin with Option

Revision ID: 1856e8cfd8d6
Revises: 
Create Date: 2017-01-25 10:06:16.194670

"""

# revision identifiers, used by Alembic.
revision = '1856e8cfd8d6'
down_revision = None
branch_labels = None
depends_on = None

from alembic import op
import sqlalchemy as sa


def upgrade():
    ### commands auto generated by Alembic - please adjust! ###
    op.create_table('option',
    sa.Column('id', sa.Integer(), nullable=False),
    sa.Column('date_created', sa.DateTime(), nullable=True),
    sa.Column('version', sa.String(), nullable=True),
    sa.Column('gameport', sa.Integer(), nullable=True),
    sa.Column('wizlock', sa.Boolean(), nullable=True),
    sa.PrimaryKeyConstraint('id')
    )
    ### end Alembic commands ###


def downgrade():
    ### commands auto generated by Alembic - please adjust! ###
    op.drop_table('option')
    ### end Alembic commands ###